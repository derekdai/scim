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
#define log_func_not_impl()                       log_trace("%s not implement yet", __FUNCTION__)
#define log_func_incomplete()                     log_trace("%s implementation incomplete", __FUNCTION__)
#define log_func_cant_mapped()                    log_trace("%s can not be mapped", __FUNCTION__)

#ifndef SD_BUS_METHOD_WITH_NAMES
#define SD_BUS_METHOD_WITH_NAMES( member, signature, in_names, result, out_names, handler, flags) \
    SD_BUS_METHOD( member, signature, result, handler, flags)
#endif

#ifndef SD_BUS_PARAM
#define SD_BUS_PARAM(name)
#endif

template<typename T, int (T::*mf)(sd_event_source *s, int fd, uint32_t revents)>
int
sd_event_io_adapter(sd_event_source *s, int fd, uint32_t revents, void *userdata)
{
    return (static_cast<T *>(userdata)->*mf)(s, fd, revents);
}

template<typename T, int (T::*mf)(sd_event_source *s)>
int
sd_event_adapter(sd_event_source *s, void *userdata)
{
    return (static_cast<T *>(userdata)->*mf)(s);
}

template<typename T, int (T::*mf)(sd_bus_message *m, sd_bus_error *ret_error)>
int
sd_bus_message_adapter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint8_t type;
    sd_bus_message_get_type(m, &type);
    log_trace("%s %s %s.%s",
            sd_bus_message_get_sender(m),
            type == SD_BUS_MESSAGE_METHOD_CALL
                ? "calls"
                : type == SD_BUS_MESSAGE_SIGNAL
                    ? "signals"
                    : "",
            sd_bus_message_get_interface(m),
            sd_bus_message_get_member(m));
    sd_bus_message_dump(m, stderr, 0);
    sd_bus_message_rewind(m, 1);

    return (static_cast<T *>(userdata)->*mf)(m, ret_error);
}

template<typename T, int (T::*mf)(sd_bus *bus,
 	                              const char *path,
 	                              const char *interface,
 	                              const char *property,
 	                              sd_bus_message *value,
 	                              sd_bus_error *ret_error)>
int
sd_bus_prop_adapter(sd_bus *bus,
 	                const char *path,
 	                const char *interface,
 	                const char *property,
 	                sd_bus_message *value,
 	                void *userdata,
 	                sd_bus_error *ret_error)
{
    return (static_cast<T *>(userdata)->*mf)(bus, path, interface, property, value, ret_error);
}

static inline uint16_t
scim_ibus_keystate_to_scim (uint32_t state)
{
    uint16_t mask = 0;

    // Check Meta mask first, because it's maybe a mask combination.
    if (state & IBUS_META_MASK) {
        mask |= scim::SCIM_KEY_MetaMask;
    }

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

    if (state & IBUS_SUPER_MASK) {
        mask |= scim::SCIM_KEY_SuperMask;
    }

    if (state & IBUS_HYPER_MASK) {
        mask |= scim::SCIM_KEY_HyperMask;
    }

    if (state & IBUS_MOD2_MASK) {
        mask |= scim::SCIM_KEY_NumLockMask;
    }

    return mask;
}

static inline scim::KeyEvent
scim_ibus_keyevent_to_scim (uint32_t keyval, uint32_t keycode, uint32_t state)
{
    scim::KeyEvent  scimkey;
//    KeySym          keysym;
//    XKeyEvent       key = xkey;
//    char            buf [32];

    scimkey.code = keyval;

    scimkey.mask = scim_ibus_keystate_to_scim (state);

    if (state & IBUS_RELEASE_MASK) scimkey.mask |= scim::SCIM_KEY_ReleaseMask;

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

    return scimkey;
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
#define ibus_caps_to_str(caps)
#endif

#endif // _SCIM_IBUS_UTILS_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
