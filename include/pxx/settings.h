/** \file pxx/settings.h
 *
 * This file defines the settings struct which holds all basic setting
 * that define the behavior of pxx.
 *
 */
#ifndef __PXX_SETTINGS_H__
#define __PXX_SETTINGS_H__

namespace pxx {

struct Settings {


    std::string header = "";
    std::vector<std::string> includes = {};


};


} // namespace pxx

#endif
