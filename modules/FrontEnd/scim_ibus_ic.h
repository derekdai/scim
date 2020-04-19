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
    IBusInputContextObserver       *m_observer;
    int                             m_id;
    int                             m_siid;
    bool                            m_shared_siid;
    bool                            m_on;
    String                          m_locale;
    String                          m_encoding;
    bool                            m_onspot_preedit_started;
    int                             m_onspot_preedit_length;
    int                             m_onspot_caret;

    bool                            m_client_commit_preedit;
    uint32_t                        m_purpose;
    uint32_t                        m_hints;
    uint32_t                        m_capabilities;
    IBusRect                        m_cursor_location;

    sd_bus                         *m_bus;
    sd_bus_slot                    *m_inputcontext_slot;
    sd_bus_slot                    *m_service_slot;
    String                          m_object_path;

    static const sd_bus_vtable      m_inputcontext_vtbl[];
    static const sd_bus_vtable      m_service_vtbl[];

public:
    IBusInputContext(IBusInputContextObserver *observer, int id, int siid);

    virtual ~IBusInputContext();

    int init(sd_bus *bus);

    int get_id() const { return m_id; }
    int get_siid() const { return m_siid; }
    void set_siid(int siid) { m_siid = siid; }
    bool is_shared_siid() const { return m_shared_siid; }
    void set_shared_siid(bool shared) { m_shared_siid = shared; }
    bool is_on() const { return m_on; }
    void set_on(bool on) { m_on = on; }
    const String get_locale() const { return m_locale; }
    const String get_encoding() const { return m_encoding; }
    bool is_onspot_preedit_started() const { return m_onspot_preedit_started; }
    void set_onspot_preedit_started(bool started) { m_onspot_preedit_started = started; }
    bool get_onspot_preedit_length() const { return m_onspot_preedit_length; }
    void set_onspot_preedit_length(int len) { m_onspot_preedit_length = len; }
    int get_onspot_caret() const { return m_onspot_caret; }
    void set_onspot_caret(int caret_pos) { m_onspot_caret = caret_pos; }
    const IBusRect &get_cursor_location() const { return m_cursor_location; }

    bool is_client_commit_preedit() const { return m_client_commit_preedit; }
    uint32_t get_capabilities() const { return m_capabilities; }

    const String &get_object_path() const { return m_object_path; }

    int notify_commit_text(const char *text);
    int notify_forward_key_event(uint32_t keyval, uint32_t keycode, uint32_t state);
    int notify_update_preedit_text(const char *text, uint32_t cursor_pos, bool visible, uint32_t mode = 0);
    int notify_show_preedit_text();
    int notify_hide_preedit_text();
    int notify_update_aux_text(const char *text, bool visible);
    int notify_show_aux_text();
    int notify_hide_aux_text();
    int notify_update_lookup_table(/* v* */ void *table, bool visible);
    int notify_show_lookup_table();
    int notify_hide_lookup_table();
    int notify_page_up_lookup_table();
    int notify_page_down_lookup_table();
    int notify_cursor_up_lookup_table();
    int notify_cursor_down_lookup_table();
    int notify_register_properties(/* v */ void *props);
    int notify_update_property(/* v */ void *prop);


private:
    /* IBus methods, signals and properties */
    int destroy(sd_bus_message *m, sd_bus_error *ret_error);
    int focus_in(sd_bus_message *m, sd_bus_error *ret_error);
    int focus_out(sd_bus_message *m, sd_bus_error *ret_error);
    int reset(sd_bus_message *m, sd_bus_error *ret_error);
    int process_key_event(sd_bus_message *m, sd_bus_error *ret_error);
    int set_cursor_location(sd_bus_message *m, sd_bus_error *ret_error);
    int set_cursor_location_relative(sd_bus_message *m, sd_bus_error *ret_error);
    int process_hand_writing_event(sd_bus_message *m, sd_bus_error *ret_error);
    int cancel_hand_writing(sd_bus_message *m, sd_bus_error *ret_error);
    int property_activate(sd_bus_message *m, sd_bus_error *ret_error);
    int set_engine(sd_bus_message *m, sd_bus_error *ret_error);
    int get_engine(sd_bus_message *m, sd_bus_error *ret_error);
    int set_surrounding_text(sd_bus_message *m, sd_bus_error *ret_error);
    int set_capabilities(sd_bus_message *m, sd_bus_error *ret_error);
    int get_client_commit_preedit(sd_bus *bus,
 	                              const char *path,
 	                              const char *interface,
 	                              const char *property,
 	                              sd_bus_message *value,
 	                              sd_bus_error *ret_error);
    int set_client_commit_preedit(sd_bus *bus,
 	                              const char *path,
 	                              const char *interface,
 	                              const char *property,
 	                              sd_bus_message *value,
 	                              sd_bus_error *ret_error);
    int get_content_type(sd_bus *bus,
 	                     const char *path,
 	                     const char *interface,
 	                     const char *property,
 	                     sd_bus_message *value,
 	                     sd_bus_error *ret_error);
    int set_content_type(sd_bus *bus,
 	                     const char *path,
 	                     const char *interface,
 	                     const char *property,
 	                     sd_bus_message *value,
 	                     sd_bus_error *ret_error);

    void deinit();
};

#endif // _SCIM_IBUS_IC_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
