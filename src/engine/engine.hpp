#ifndef __nobb__engine__engine__hpp
#define __nobb__engine__engine__hpp

#include "engine.h"
#include "molecule/molecule.h"

// Methods

template <typename type, typename lambda, typename std :: enable_if <std :: is_same <type, molecule> :: value> :: type *> void engine :: each(const lambda & callback) const
{
  this->_molecules.each([&](molecule * molecule)
  {
    molecule->integrate(this->_time);
    callback((const class molecule &) (*molecule));
  });
}

template <typename type, typename lambda, typename std :: enable_if <std :: is_same <type, molecule> :: value> :: type *> void engine :: each(const uint8_t & tag, const lambda & callback) const
{
  this->_tags[tag].each([&](molecule * molecule)
  {
    molecule->integrate(this->_time);
    callback((const class molecule &) (*molecule));
  });
}

template <typename type, typename lambda, typename std :: enable_if <std :: is_same <type, bumper> :: value> :: type *> void engine :: each(const lambda & callback) const
{
  this->_bumpers.each([&](bumper * bumper)
  {
    callback((const class bumper &) (*bumper));
  });
}

template <typename etype, typename lambda, typename std :: enable_if <std :: is_same <etype, events :: molecule> :: value> :: type *> size_t engine :: on(const lambda & callback)
{
  :: callback <events :: molecule> * wrapper = new :: callback <events :: molecule, lambda> (callback);
  return this->_dispatcher.add(wrapper);
}

template <typename etype, typename lambda, typename std :: enable_if <std :: is_same <etype, events :: bumper> :: value> :: type *> size_t engine :: on(const lambda & callback)
{
  :: callback <events :: bumper> * wrapper = new :: callback <events :: bumper, lambda> (callback);
  return this->_dispatcher.add(wrapper);
}


#endif