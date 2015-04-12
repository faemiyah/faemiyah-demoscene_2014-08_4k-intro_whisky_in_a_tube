#include "image_png.hpp"
#include "intro.hpp"

#include <iostream>
#include <sstream>
#include <stdio.h>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>
#include <boost/tuple/tuple.hpp>

#include "GL/glew.h"

#if defined(__APPLE__)
#define MAINPROG SDL_main
#else
#define MAINPROG main
#endif

namespace po = boost::program_options;

/** Console output content. */
static const char *usage = ""
"Usage: whisky_in_a_tube [options]\n"
"For Assembly 2014 4k intro compo.\n"
"Release version does not pertain to any size limitations.\n"
"\n";

/** \brief Audio writing callback.
 *
 * \param data Raw audio data.
 * \param size Audio data size (in samples).
 */
void write_audio_callback(void *data, unsigned size)
{
  FILE *fd = fopen("whisky_in_a_tube.raw", "wb");

  if(fd != NULL)
  {
    fwrite(data, size, 1, fd);
  }

  fclose(fd);
  return;
}

/** \brief Image writing callback.
 *
 * \param screen_w Screen width.
 * \param screen_h Screen height.
 * \param idx Frame index to write.
 */
void write_frame_callback(unsigned screen_w, unsigned screen_h, unsigned idx)
{
  boost::scoped_array<uint8_t> image(new uint8_t[screen_w * screen_h * 3]);
  char filename[25];

  glReadPixels(0, 0, static_cast<GLsizei>(screen_w), static_cast<GLsizei>(screen_h), GL_RGB, GL_UNSIGNED_BYTE,
      image.get());

  sprintf(filename, "whisky_in_a_tube_%04u.png", idx);

  gfx::image_png_save(std::string(filename), screen_w, screen_h, 24, image.get());
  return;
}

/** \brief Parse resolution from string input.
 *
 * \param op Resolution string.
 * \return Tuple of width and height.
 */
boost::tuple<int, int> parse_resolution(const std::string &op)
{
  size_t cx = op.find("x");
  int width;
  int height;

  if(std::string::npos != cx)
  {
    std::string width_string = op.substr(0, cx);
    std::string height_string = op.substr(cx + 1);

    width = boost::lexical_cast<int>(width_string);
    height = boost::lexical_cast<int>(height_string);

    if((0 >= width) || (0 >= height))
    {
      std::ostringstream sstr;
      sstr << "invalid width x height in resolution string '" << op << '\'';
      BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
    }
  }
  else
  {
    cx = op.find("p");
  
    if(std::string::npos == cx)
    {
      std::ostringstream sstr;
      sstr << "invalid resolution string '" << op << '\'';
      BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
    }
    
    std::string progressive = op.substr(0, cx);
    height = boost::lexical_cast<int>(op.substr(0, cx));

    switch(height)
    {
      case 1080:
        width = 1920;
        break;

      case 720:
        width = 1280;
        break;

      default:
        {
          std::ostringstream sstr;
          sstr << "invalid progressive mode identifier in resolution string '" << op << '\'';
          BOOST_THROW_EXCEPTION(std::runtime_error(sstr.str()));
        }
        break;
    }
  }

  return boost::make_tuple(width, height);
}

extern "C"
int MAINPROG(int argc, char **argv)
{
  unsigned screen_w = 1280;
  unsigned screen_h = 720;
  bool developer = false;
  bool fullscreen = true;
  bool record = false;

  try
  {
    if(argc > 0)
    {
      po::options_description desc("Options");
      desc.add_options()
        ("developer,d", "Developer mode.")
        ("help,h", "Print help text.")
        ("record,R", "Do not play intro normally, instead save audio as .wav and frames as .png -files.")
        ("resolution,r", po::value<std::string>(), "Resolution to use, specify as 'WIDTHxHEIGHT'.")
        ("window,w", "Start in window instead of full-screen.");

      po::variables_map vmap;
      po::store(po::command_line_parser(argc, argv).options(desc).run(), vmap);
      po::notify(vmap);

      if(vmap.count("developer"))
      {
        developer = true;
      }
      if(vmap.count("help"))
      {
        std::cout << usage << desc << std::endl;
        return 0;
      }
      if(vmap.count("record"))
      {
        record = true;
      }
      if(vmap.count("resolution"))
      {
        boost::tie(screen_w, screen_h) = parse_resolution(vmap["resolution"].as<std::string>());
      }
      if(vmap.count("window"))
      {
        fullscreen = false;
      }
    }

    return intro(screen_w, screen_h, developer, fullscreen, record);
  }
  catch(const boost::exception &ee)
  {
    std::cerr<< boost::diagnostic_information(ee);
    return EXIT_FAILURE;
  }
}

