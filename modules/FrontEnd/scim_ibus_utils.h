/** @file scim_ibus_utils.h
 * definition of IBUSUTILS related classes.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Publutils
 * Lutilsense as published by the Free Software Foundation; either
 * version 2 of the Lutilsense, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTUTILSULAR PURPOSE.  See the
 * GNU Lesser General Publutils Lutilsense for more details.
 *
 * You should have received a copy of the GNU Lesser General Publutils
 * Lutilsense along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_ibus_utils.h,v 1.10 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_UTILS_H)
#define __SCIM_IBUS_UTILS_H

enum {
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
};
#define LOG_LEVEL LOG_LEVEL_TRACE
#define log_level_enabled(l) ((l) >= LOG_LEVEL)
#define log_print(level, fmt, ...) ({                                                               \
    int l = (level);                                                                                \
    if (log_level_enabled(l)) {                                                                     \
        static const char *names[] = { "TRC", "DBG", "INF", "WRN", "ERR", "FAL" };                  \
        fprintf(stderr, "%s " __FILE__ ":%-4d " fmt "\n", names[(l)], __LINE__, ##__VA_ARGS__);     \
    }                                                                                               \
})
#define log_trace(fmt, ...)                       log_print(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)                       log_print(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)                        log_print(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)                        log_print(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)                       log_print(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)                       log_print(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define log_func()                                log_trace("%s", __FUNCTION__)
#define log_func_not_impl(...)                    ({ log_error("%s not implement yet", __FUNCTION__); return __VA_ARGS__; })
#define log_func_incomplete(...)                  ({ log_error("%s implementation incomplete", __FUNCTION__); return __VA_ARGS__; })
#define log_func_ignored(...)                     ({ log_trace("%s ignored", __FUNCTION__); return __VA_ARGS__; })

#ifndef SD_BUS_METHOD_WITH_NAMES
#define SD_BUS_METHOD_WITH_NAMES( member, signature, in_names, result, out_names, handler, flags) \
    SD_BUS_METHOD( member, signature, result, handler, flags)
#endif

#ifndef SD_BUS_PARAM
#define SD_BUS_PARAM(name)
#endif

static inline uint16_t
scim_ibus_keystate_to_scim_keymask (uint32_t state)
{
    uint16_t mask = 0;

    if (state & IBUS_SHIFT_MASK) {
        mask |= scim::SCIM_KEY_ShiftMask;
    }

    if (state & IBUS_LOCK_MASK) {
        mask |= scim::SCIM_KEY_CapsLockMask;
    }

    if (state & IBUS_CONTROL_MASK) {
        mask |= scim::SCIM_KEY_ControlMask;
    }

    if (state & IBUS_MOD1_MASK) {
        mask |= scim::SCIM_KEY_AltMask;
    }

    if (state & IBUS_META_MASK) {
        mask |= scim::SCIM_KEY_MetaMask;
    }

    if (state & IBUS_SUPER_MASK) {
        mask |= scim::SCIM_KEY_SuperMask;
    }

    if (state & IBUS_HYPER_MASK) {
        mask |= scim::SCIM_KEY_HyperMask;
    }

    if (state & IBUS_MOD2_MASK) {
        mask |= scim::SCIM_KEY_NumLockMask;
    }

    if (state & IBUS_RELEASE_MASK) {
        mask |= SCIM_KEY_ReleaseMask;
    }

    return mask;
}

static inline std::string
scim_caps_to_str (uint32_t caps)
{
    std::ostringstream ss;

    if (caps & SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT)
    {
        ss << "onthespo-preedit|";
    }
    if (caps & SCIM_CLIENT_CAP_SINGLE_LEVEL_PROPERTY)
    {
        ss << "single-level-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_MULTI_LEVEL_PROPERTY)
    {
        ss << "multi-level-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_TRIGGER_PROPERTY)
    {
        ss << "trigger-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_HELPER_MODULE)
    {
        ss << "helper-module|";
    }
    if (caps & SCIM_CLIENT_CAP_SURROUNDING_TEXT)
    {
        ss << "surrounding-text|";
    }

    std::string s = ss.str();
    if (s.size() > 0) {
        s.resize(s.size() - 1);
    }

    return s;
}

static inline scim::KeyEvent
scim_ibus_keyevent_to_scim_keyevent (KeyboardLayout layout,
                                     uint32_t keyval,
                                     uint32_t keycode,
                                     uint32_t state)
{
     return KeyEvent (keyval,
                      scim_ibus_keystate_to_scim_keymask (state),
                      layout);

//    if (scimkey.code == SCIM_KEY_backslash) {
//        int keysym_size = 0;
//        KeySym *keysyms = XGetKeyboardMapping (display, xkey.keycode, 1, &keysym_size);
//        if (keysyms != NULL) {
//            if (keysyms[0] == XK_backslash &&
//		(keysym_size > 1 && keysyms[1] == XK_underscore))
//                scimkey.mask |= SCIM_KEY_QuirkKanaRoMask;
//            XFree (keysyms);
//        }
//    }
}

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
static inline std::string ibus_caps_to_str(uint32_t caps)
{
    std::ostringstream ss;
    if (caps & IBUS_CAP_PREEDIT_TEXT) {
        ss << "preedit-text|";
    }
    if (caps & IBUS_CAP_AUXILIARY_TEXT) {
        ss << "aux-text|";
    }
    if (caps & IBUS_CAP_LOOKUP_TABLE) {
        ss << "lookup-table|";
    }
    if (caps & IBUS_CAP_FOCUS) {
        ss << "focus|";
    }
    if (caps & IBUS_CAP_PROPERTY) {
        ss << "property|";
    }
    if (caps & IBUS_CAP_SURROUNDING_TEXT) {
        ss << "surrounding-text|";
    }

    std::string s = ss.str();
    if (s.size() > 0) {
        s.resize(s.size() - 1);
    }

    return s;
}
#else
#define ibus_caps_to_str(caps) (String())
#endif

#endif // _SCIM_IBUS_UTILS_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
