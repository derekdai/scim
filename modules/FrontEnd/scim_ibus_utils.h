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
#define log_print(level, fmt, ...) ({                                           \
    if ((level) >= LOG_LEVEL) {                                                 \
        fprintf(stderr, __FILE__ ":%-4d " fmt "\n", __LINE__, ##__VA_ARGS__);   \
    }                                                                           \
})
#define log_trace(fmt, ...)                       log_print(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)                       log_print(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)                        log_print(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)                        log_print(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)                       log_print(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)                       log_print(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define log_func()                                log_trace("%s", __FUNCTION__)

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

template<typename T, int (T::*mf)(sd_bus_message *m, sd_bus_error *ret_error)>
int
sd_bus_message_adapter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return (static_cast<T *>(userdata)->*mf)(m, ret_error);
}

template<typename T, int (T::*mf)(sd_event_source *s)>
int
sd_event_adapter(sd_event_source *s, void *userdata)
{
    return (static_cast<T *>(userdata)->*mf)(s);
}

#endif // _SCIM_IBUS_UTILS_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
