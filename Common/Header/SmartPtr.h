/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: SmartPtr.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __SMARTPTR_H__
#define __SMARTPTR_H__


/** 
 * @brief Reference counting based smart pointer.
 */
template<class T>
class CSmartPtr {
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

#endif /* __SMARTPTR_H__ */
