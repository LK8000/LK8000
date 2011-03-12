/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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


/** 
 * @brief Pointers compare object
 */
template<class T>
struct CPtrCmp {
  bool operator()(const T &left, const T &right) const { return *left < *right; }
};


/** 
 * @brief Reference counting based smart pointer.
 */
template<class T>
struct CSmartPtr {
  T * const _ptr;
  unsigned *_refNum;
  CSmartPtr &operator=(const CSmartPtr &) const;
public:
  CSmartPtr(T *ptr): _ptr(ptr), _refNum(new unsigned(1)) {}
  CSmartPtr(const CSmartPtr &ref):
    _ptr(ref._ptr), _refNum(ref._refNum)
  {
    (*_refNum)++;
  }
  ~CSmartPtr()
  {
    if(!--(*_refNum)) {
      delete _refNum;
      delete _ptr;
    }
  }
  T &operator*()              { return *_ptr; }
  const T &operator*() const  { return *_ptr; }
  T *operator->()             { return _ptr; }
  const T *operator->() const { return _ptr; }
};


std::string TimeToString(unsigned time);
std::string CoordToString(double coord, bool latitude);


#endif /* __TOOLS_H__ */
