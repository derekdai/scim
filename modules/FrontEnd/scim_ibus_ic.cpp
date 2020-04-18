/** @file scim_ibus_frontend.cpp
 * implementation of class IBusFrontEnd.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_ibus_frontend.cpp,v 1.179.2.6 2007/06/16 06:23:38 suzhe Exp $
 *
 */
#include <systemd/sd-bus.h>
#include <sstream>
#include "scim_ibus_utils.h"
#include "scim_ibus_types.h"
#include "scim_ibus_ic.h"

#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"


using namespace std;

template<int (IBusInputContext::*mf)(sd_bus_message *m, sd_bus_error *ret_error)>
int
ibus_inputcontext_message_adapter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_message_adapter<IBusInputContext, mf>(m, userdata, ret_error);
}

template<int (IBusInputContext::*mf)(sd_event_source *s)>
int
ibus_inputcontext_event_adapter(sd_event_source *s, void *userdata)
{
    return sd_event_adapter<IBusInputContext, mf>(s, userdata);
}

const sd_bus_vtable IBusInputContext::m_inputcontext_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            "b", SD_BUS_PARAM(handled),
            (&ibus_inputcontext_message_adapter<&IBusInputContext::process_key_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusInputContext::m_service_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD(
            "Destroy",
            "",
            "",
            (&ibus_inputcontext_message_adapter<&IBusInputContext::destroy>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

IBusInputContext::IBusInputContext(IBusPortal *portal, int id, int siid)
    : m_id(id),
      m_siid(siid),
      m_portal(portal),
      m_inputcontext_slot(NULL),
      m_service_slot(NULL)
{
    log_func();
}

IBusInputContext::~IBusInputContext()
{
    log_func();

    deinit();
}

int IBusInputContext::init(sd_bus *bus)
{
    log_func();

    ostringstream ss;
    ss << IBUS_INPUTCONTEXT_OBJECT_PATH << m_id;
    String path = ss.str();

    log_debug("object path: %s", path.c_str());

    if (sd_bus_add_object_vtable(bus,
                                 &m_service_slot,
                                 path.c_str(),
                                 IBUS_SERVICE_INTERFACE,
                                 m_service_vtbl,
                                 this) < 0) {
        return -1;
    }

    if (sd_bus_add_object_vtable(bus,
                                 &m_inputcontext_slot,
                                 path.c_str(),
                                 IBUS_INPUTCONTEXT_INTERFACE,
                                 m_inputcontext_vtbl,
                                 this) < 0) {
        return -1;
    }

    m_object_path = path;

    return 0;
}

void IBusInputContext::deinit()
{
    log_func();

    if(m_inputcontext_slot) {
        sd_bus_slot_unref(m_inputcontext_slot);
        m_inputcontext_slot = NULL;
    }

    if(m_service_slot) {
        sd_bus_slot_unref(m_service_slot);
        m_service_slot = NULL;
    }
}

int IBusInputContext::destroy(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    deinit();

    m_portal->destroy_input_context(m_id);

    return 0;
}

int IBusInputContext::process_key_event(sd_bus_message *m, sd_bus_error *ret_error)
{
    log_func();

    return 0;
}
