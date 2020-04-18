/** @file scim_ibus_ic.h
 * definition of IBUSIC related classes.
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
 * $Id: scim_ibus_ic.h,v 1.10 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_IC_H)
#define __SCIM_IBUS_IC_H

#include "scim.h"

using namespace scim;

class IBusPortal;

class IBusInputContext: public ReferencedObject
{
    IBusPortal                     *m_portal;
    int                             m_id;
    int                             m_siid;
    bool                            m_shared_siid;
    String                          m_object_path;

    sd_bus_slot                    *m_inputcontext_slot;
    sd_bus_slot                    *m_service_slot;

    static const sd_bus_vtable      m_inputcontext_vtbl[];
    static const sd_bus_vtable      m_service_vtbl[];

public:
    IBusInputContext(IBusPortal *portal, int id, int siid);

    virtual ~IBusInputContext();

    int init(sd_bus *bus);

    int get_id() const { return m_id; }
    int get_siid() const { return m_siid; }
    bool is_shared_siid() { return m_shared_siid; }
    void set_shared_siid(bool shared) { m_shared_siid = shared; }
    const String get_object_path() const { return m_object_path; }

private:
    int destroy(sd_bus_message *m, sd_bus_error *ret_error);
    int process_key_event(sd_bus_message *m, sd_bus_error *ret_error);

    void deinit();
};

typedef Pointer <IBusInputContext> IBusInputContextPointer;

#endif // _SCIM_IBUS_IC_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
