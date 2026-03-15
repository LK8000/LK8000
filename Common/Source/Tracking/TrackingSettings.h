/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef TRACKING_TRACKINGSETTINGS_H
#define TRACKING_TRACKINGSETTINGS_H

#include <vector>

namespace settings {
class writer;
}

namespace tracking {
struct Profile;

namespace settings_io {

bool LoadProfileSettings(const char* key, const char* value,
                         std::vector<Profile>& profiles);

void SaveProfileSettings(settings::writer& writer_settings,
                         const std::vector<Profile>& profiles);

}  // namespace settings_io
}  // namespace tracking

#endif  // TRACKING_TRACKINGSETTINGS_H
