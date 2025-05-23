/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_IO_DATA_HANDLER_HPP
#define XCSOAR_IO_DATA_HANDLER_HPP

#include "Compiler.h"
#include <vector>
#include <stddef.h>
#include "utils/uuid.h"

/**
 * Interface with callbacks for the #Port class.
 */
class DataHandler {
public:
  gcc_nonnull_all
  virtual void DataReceived(const void *data, size_t length) { };

  gcc_nonnull_all
  virtual void OnCharacteristicChanged(uuid_t service, uuid_t characteristic, std::vector<uint8_t>&& data) { }

  virtual bool DoEnableNotification(const uuid_t& service, const uuid_t& characteristic) const {
    return false;
  }
};

#endif
