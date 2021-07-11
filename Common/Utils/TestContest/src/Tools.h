/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <sstream>
#include <stdexcept>


/**
 * @brief Converts string to specific type.
 *
 * Function converts provided string to specified type. The convertion
 * is being done using STL streams so the output type have to provide
 * the means to initialize itself from the stream.
 *
 * @param str The string to convert.
 *
 * @exception std Thrown when operation failed.
 *
 * @return The data of specified type.
 */
template<class T>
T Convert(const std::string &str)
{
  T value;
  std::stringstream stream(str);
  stream >> value;
  if(stream.fail() && !stream.eof())
    throw std::runtime_error("Cannot convert '" + str + "' to requested type!!!");
  return value;
}


/**
 * @brief Converts any type to a string.
 *
 * Function converts provided data to a string. The convertion
 * is being done using STL streams so the input type have to provide
 * the means to convert itself into the stream.
 *
 * @param val The value to convert.
 *
 * @exception std Thrown when operation failed.
 *
 * @return The string describing provided data.
 */
template<class T>
std::string Convert(const T &val)
{
  std::stringstream stream;
  stream << val;
  return stream.str();
}


/**
* @brief Clears STL sequence container
* 
* Deletes all pointers and clears STL sequence container.
* 
* @param container STL sequence container to clear
*/
template<class Seq> void Purge(Seq &container)
{
  typename Seq::iterator it;
  for(it = container.begin(); it != container.end(); ++it)
    delete *it;
  container.clear();
}


std::string TimeToString(unsigned time);
std::string CoordToString(double coord, bool latitude);


#endif /* __TOOLS_H__ */
