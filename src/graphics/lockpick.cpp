#include "lockpick.h"

bool __enter__ = false;
bool __click__ = false;

namespace lockpick
{
  // vector

  // Constructors

  vector :: vector()
  {
    this->x = 0.;
    this->y = 0.;
  }

  vector :: vector(double x, double y)
  {
    this->x = x;
    this->y = y;
  }

  // color

  // Constructors

  color :: color()
  {
  }

  color :: color(unsigned char red, unsigned char green, unsigned char blue)
  {
    this->red = red;
    this->green = green;
    this->blue = blue;
  }

  // glcircle

  // Constructors

  window :: _glcircle :: _glcircle() : buf(nullptr)
  {
  }

  window :: _glcircle :: _glcircle(unsigned int subdivs) : subdivs(subdivs + 2)
  {
    this->buf = new double[this->subdivs];
    this->buf[0] = 0.;
    this->buf[1] = 0.;
    for (unsigned int i = 0; i <= subdivs; i++)
    {
      double angle = i * ((2. * M_PI) / subdivs);
      this->buf[i + 2] = std :: cos(angle);
      this->buf[i + 3] = std :: sin(angle);
    }
  }

  // Destructor

  window :: _glcircle :: ~_glcircle()
  {
    delete[] this->buf;
  }

  // Operators

  window :: _glcircle & window :: _glcircle :: operator=(window :: _glcircle g)
  {
    this->subdivs = g.subdivs;
    this->buf = new double[this->subdivs];
    std :: memcpy(this->buf, g.buf, sizeof(double)*this->subdivs);
    return *this;
  }


  // circle

  // Constructors

  window :: _circle :: _circle()
  {
    _size = 0.;
    _x = 0.;
    _y = 0.;
    _scale = 0.;
  }

  window :: _circle :: _circle(vector pos, double scale, color c, unsigned int size) : _size(size), _c(c), _scale(scale)
  {
    _x = pos.x;
    _y = pos.y;
  }

  window :: _circle :: _circle(unsigned int size) : _size(size)
  {
    _x = 0.;
    _y = 0.;
    _scale = 0.;
  }

  // Members

  void window :: _circle :: draw(__unused vector center, __unused double scale, __unused color c)
  {
#ifdef __graphics__
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                         // To operate on model-view matrix
                                                        // Clear window and null buffer Z
                                                        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                                        // Reset transformation
    glLoadIdentity();

    //glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    glTranslated( GLdouble(center.x), GLdouble(center.y), GLdouble(0.) );
    glScaled( GLdouble(scale), GLdouble(scale), GLdouble(0.) );
    glColor3ub( c.red, c.green, c.blue );
    glDrawArrays( GL_TRIANGLE_FAN, 0, _size );
    glPopMatrix();
#endif
  }

  // window

  // Constructors

  window :: window(__unused const char * title, __unused int width, __unused int height, __unused int position_x, __unused int position_y, __unused color background, __unused int frame_width_percentage, __unused int frame_height_percentage)
  {
#ifdef __graphics__
    int myargc = 1;
    char * myargv[1];
    myargv[0] = strdup("nocs");
    glutInit(&myargc, myargv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(600, 600);
    glutCreateWindow(__default_title);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.f, 1.f, 0.f, 1.f); // xmin, xmax, ymin, ymax
    glutDisplayFunc(dummy_flush);
    glutKeyboardFunc([](unsigned char key, __unused int x, __unused int y) {
      switch (key)
      {
      case 32:
        __enter__ = true;
        break;
      default:
        __enter__ = false;
        break;
      }
    });
    glutMouseFunc([](int button, __unused int state, __unused int x, __unused int y) {
      switch (button)
      {
      case GLUT_LEFT_BUTTON:
        __click__ = true;
        break;
      default:
        __click__ = false;
        break;
      }
    });
    // Set range coordinates
    
    start();
#endif
  }

  // Destructor

  window :: ~window()
  {
#ifdef __graphics__
    _th.join();
    --__window_count;
    _id = glutGetWindow();
    glutLeaveMainLoop();

    if (__window_count == 0)
    {
      glutDestroyWindow(_id);
      __started = false;
    }
#endif
  }

  // Methods

  void window :: line(__unused vector from, __unused vector to)
  {
#ifdef __graphics__
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                         // To operate on model-view matrix
                                                        // Clear window and null buffer Z
                                                        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                                        // Reset transformation
    //glLoadIdentity();

    //glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_LINE_STRIP);
    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(GLdouble(from.x), GLdouble(from.y), GLdouble(0.));
    glVertex3d(GLdouble(to.x), GLdouble(to.y), GLdouble(0.));
    glEnd();
    glPopMatrix();
#endif
  }

  void window :: circle(__unused vector center, __unused double radius, __unused color c)
  {
#ifdef __graphics__
    int i;
    int triangleAmount = 20; //# of triangles used to draw circle

    //GLfloat radius = 0.8f; //radius
    GLfloat twicePi = 2.0f * M_PI;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center.x, center.y); // center of circle
    for (i = 0; i <= triangleAmount; i++)
    {
      glVertex2f(
          center.x + (radius * cos(i * twicePi / triangleAmount)),
          center.y + (radius * sin(i * twicePi / triangleAmount)));
    }
    glEnd();
    
#endif
  }


  // Static methods

  void window :: wait_enter()
  {
  #ifdef __graphics__
    __enter__ = false;
    while (true)
    {
      if (__enter__) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  #endif
  }

  void window :: wait_click()
  {
  #ifdef __graphics__
    __click__ = false;
    while (true)
    {
      if (__click__) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  #endif
  }

  void window :: flush()
  {
#ifdef __graphics__
    glutSwapBuffers();
#endif
  }

  void window :: dummy_flush()
  {}

  void window :: clear()
  {
#ifdef __graphics__
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT); // Clean the screen and the depth buffer
    //glLoadIdentity();                                   // Reset the projection matrix
#endif
  }

  // Static private methods

  void window :: start()
  {
#ifdef __graphics__
    if (!__started)
    {
      __started = true;
      _th = std::thread(&window :: run, this);
    }
#endif
  }

  void window :: run()
  {
#ifdef __graphics__
    // Control OpenGL events
    glutMainLoop();                                    // Enter the infinite event-processing loop
#endif
  }


  // Static members

  bool window :: __started = false;
  int window :: __window_count = 0;

}
