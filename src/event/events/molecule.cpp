#include "molecule.hpp"
#include "engine/engine.h"
#include "callback/dispatcher.h"

namespace events
{
  // Constructors

  molecule :: molecule(:: molecule & alpha, const int & fold, :: molecule & beta, const double & elasticity)
  {
    vec xa = alpha.position() + vec(fold);
    vec xb = beta.position();

    vec va = alpha.velocity();
    vec vb = beta.velocity();

    double radiisquared = (alpha.radius() + beta.radius()) * (alpha.radius() + beta.radius());

    if(alpha.time() > beta.time())
      xb += vb * (alpha.time() - beta.time());
    else
      xa += va * (beta.time() - alpha.time());

    double time = std :: max(alpha.time(), beta.time());

    bool close;
    double beg = 0.;
    double end;

    double a = ~(va - vb);
    double b = 2. * (xa - xb) * (va - vb);
    double c = ~(xa - xb) - radiisquared;


    if(c < 0)
    {
      // Molecules are already close

      close = true;
      beg = time;
    }
    else
    {
      close = false;

      // Test 1: molecules approaching

      if(b >= 0)
      {
        this->_happens = false;
        return;
      }

      // Test 2: collision range

      double beg = newton :: quadratic(a, b, c, time);

      if(std :: isnan(beg))
      {
        this->_happens = false;
        return;
      }
    }

    // Determine end of collision range

    double mid = time + sqrt(~(xa - xb) / ~(va - vb)); // Maximum proximity of molecules centers of mass
    double beyond = 2. * mid - beg; // Parabolas are symmetric

    end = close ? (time + newton :: quadratic(a, b, c, beyond - time)) : beyond;

    double step = 0.5 * std :: min(std :: min(M_PI / fabs(alpha.angular_velocity() + beta.angular_velocity()), M_PI / fabs(alpha.angular_velocity() - beta.angular_velocity())), std :: min(M_PI / fabs(2. * alpha.angular_velocity()), M_PI / fabs(2. * beta.angular_velocity())));
    // TODO: Find out better euristics for maximum cropping of minima and maxima?

    // Look for collisions between atoms

    this->_time = std :: numeric_limits <double> :: infinity();

    for(double binbeg = beg; binbeg < end; binbeg += step)
    {
      double binend = std :: min(binbeg + step, end);

      for(size_t i = 0; i < alpha.size(); i++)
        for(size_t j = 0; j < beta.size(); j++)
        {
          double ctime = collision(alpha, i, beta, j, binbeg, binend, fold);

          if(!std :: isnan(ctime) && ctime < this->_time)
          {
            this->_time = ctime;
            this->_alpha.atom = i;
            this->_beta.atom = j;
          }
        }

      if(this->_time < std :: numeric_limits <double> :: infinity())
      {
        this->_happens = true;
        this->_alpha.molecule = &alpha;
        this->_alpha.version = alpha.version();
        this->_alpha.fold = fold;
        this->_beta.molecule = &beta;
        this->_beta.version = beta.version();

        this->_elasticity = elasticity;

        return;
      }
    }

    this->_happens = false;
  }

  // Getters

  const :: molecule & molecule :: alpha() const
  {
    return *(this->_alpha.molecule);
  }

  const :: molecule & molecule :: beta() const
  {
    return *(this->_beta.molecule);
  }

  // Methods

  bool molecule :: current()
  {
    return static_cast<int32_t>(this->_alpha.version) == this->_alpha.molecule->version() && static_cast<int32_t>(this->_beta.version) == this->_beta.molecule->version();
  }

  bool molecule :: resolve()
  {
    // Check version

    if(!current())
      return false;

    // Integrate to collision

    this->_alpha.molecule->integrate(this->_time);
    this->_beta.molecule->integrate(this->_time);

    // Update version

    (*(this->_alpha.molecule))++;
    (*(this->_beta.molecule))++;

    // Collision resolution

    vec a = position(*(this->_alpha.molecule), this->_alpha.atom, this->_alpha.fold);
    vec b = position(*(this->_beta.molecule), this->_beta.atom);

    vec n = (b - a).normalize(); // Versor of the impulse from alpha to beta

    this->v1 = this->_alpha.molecule->velocity();
    this->v2 = this->_beta.molecule->velocity();

    this->av1 = this->_alpha.molecule->angular_velocity();
    this->av2 = this->_beta.molecule->angular_velocity();

    double m1 = this->_alpha.molecule->mass();
    double m2 = this->_beta.molecule->mass();

    double i1 = this->_alpha.molecule->inertia_moment();
    double i2 = this->_beta.molecule->inertia_moment();

    this->l1 = this->av1 * i1;
    this->l2 = this->av2 * i2;

    this->p1 = m1 * this->v1;
    this->p2 = m2 * this->v2;

    this->r1 = (*(this->_alpha.molecule))[this->_alpha.atom].position() % this->_alpha.molecule->orientation() + (*(this->_alpha.molecule))[this->_alpha.atom].radius() * n;
    this->r2 = (*(this->_beta.molecule))[this->_beta.atom].position() % this->_beta.molecule->orientation() - (*(this->_beta.molecule))[this->_beta.atom].radius() * n;

    this->module = (1. + this->_elasticity) * (-(p1 * n) / (m1) + (p2 * n) / (m2) - (l1 * (r1 ^ n)) / (i1) + (l2 * (r2 ^ n)) / (i2)) / ((1 / m1) + (1 / m2) + (r1 ^ n) * (r1 ^ n) / (i1) + (r2 ^ n) * (r2 ^ n) / (i2)); // Module of the impulse

    // Update molecules' velocity and angular_velocity

    this->_alpha.molecule->impulse(r1, module * n);
    this->_beta.molecule->impulse(r2, -module * n);

    return true;
  }

  void molecule :: each(engine * engine, void (engine :: * callback)(:: molecule &, const size_t &))
  {
    (engine->*callback)(*(this->_alpha.molecule), 0);
    (engine->*callback)(*(this->_beta.molecule), this->_alpha.molecule->tag.id());
  }

  void molecule :: callback(dispatcher & dispatcher)
  {
    dispatcher.trigger(*this);
  }

  // Private methods

  std :: ostream & molecule :: print(std :: ostream & out) const
  {
    out << "Event molecule_molecule at time: " << this->_time;
    return out;
  }

  double molecule :: collision(const :: molecule & alpha, const size_t & index_alpha, const :: molecule & beta, const size_t & index_beta, const double & beg, const double & end, const int & fold)
  {
    double radiisquared = (alpha[index_alpha].radius() + beta[index_beta].radius()) * (alpha[index_alpha].radius() + beta[index_beta].radius());

    auto distsquared = [&](const double & time)
    {
      return ~(position(alpha, index_alpha, time, fold) - position(beta, index_beta, time)) - radiisquared;
    };

    double binmin = gss :: min(distsquared, beg, end);

    if(distsquared(binmin) > 0)
      return NAN;

    double binmax = gss :: max(distsquared, beg, binmin);
    double zero = secant :: compute(distsquared, binmax, binmin);

    return (distsquared(zero + time_epsilon) < distsquared(zero)) ? zero : NAN;
  }
}
