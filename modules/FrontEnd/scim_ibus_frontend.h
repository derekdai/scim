/**
 * @file scim_ibus_frontend.h
 * @brief definition of IBusFrontEnd related classes.
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
 * $Id: scim_ibus_frontend.h,v 1.26 2005/04/14 17:01:56 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_FRONTEND_H)
#define __SCIM_IBUS_FRONTEND_H

#include "scim_stl_map.h"

using namespace scim;

class IBusFrontEnd : public FrontEndBase
{
    enum ClientType {
        UNKNOWN_CLIENT,
        IMENGINE_CLIENT,
        CONFIG_CLIENT
    };

    struct ClientInfo {
        uint32     key;
        ClientType type;
    };

    /**
     * ::first = ibus id, ::second = instance id.
     */
    typedef std::vector <std::pair <int, int> > IBusInstanceRepository;

#if SCIM_USE_STL_EXT_HASH_MAP
    typedef __gnu_cxx::hash_map <int, ClientInfo, __gnu_cxx::hash <int> >   IBusClientRepository;
#elif SCIM_USE_STL_HASH_MAP
    typedef std::hash_map <int, ClientInfo, std::hash <int> >               IBusClientRepository;
#else
    typedef std::map <int, ClientInfo>                                      IBusClientRepository;
#endif

    ConfigPointer     m_config;

    SocketServer      m_socket_server;

    Transaction       m_send_trans;
    Transaction       m_receive_trans;
    Transaction       m_temp_trans;

    IBusInstanceRepository m_socket_instance_repository;

    IBusClientRepository   m_socket_client_repository;

    bool   m_stay;

    bool   m_config_readonly;

    int    m_socket_timeout;

    int    m_current_instance;

    int    m_current_socket_client;

    uint32 m_current_socket_client_key;

    int    m_ctx_counter;

    sd_event         *m_loop; 
    sd_bus           *m_bus; 
    sd_bus_slot      *m_portal_slot;

    static const sd_bus_vtable m_portal_vtbl[];
    static const sd_bus_vtable m_inputcontext_vtbl[];
    static const sd_bus_vtable m_service_vtbl[];

public:
    IBusFrontEnd (const BackEndPointer &backend,
                    const ConfigPointer  &config);

    virtual ~IBusFrontEnd ();

protected:
    virtual void show_preedit_string     (int id);
    virtual void show_aux_string         (int id);
    virtual void show_lookup_table       (int id);

    virtual void hide_preedit_string     (int id);
    virtual void hide_aux_string         (int id);
    virtual void hide_lookup_table       (int id);

    virtual void update_preedit_caret    (int id, int caret);
    virtual void update_preedit_string   (int id, const WideString & str, const AttributeList & attrs);
    virtual void update_aux_string       (int id, const WideString & str, const AttributeList & attrs);
    virtual void commit_string           (int id, const WideString & str);
    virtual void forward_key_event       (int id, const KeyEvent & key);
    virtual void update_lookup_table     (int id, const LookupTable & table);

    virtual void register_properties     (int id, const PropertyList & properties);
    virtual void update_property         (int id, const Property & property);

    virtual void beep                    (int id);

    virtual void start_helper            (int id, const String &helper_uuid);
    virtual void stop_helper             (int id, const String &helper_uuid);
    virtual void send_helper_event       (int id, const String &helper_uuid, const Transaction &trans);

    virtual bool get_surrounding_text    (int id, WideString &text, int &cursor, int maxlen_before, int maxlen_after);
    virtual bool delete_surrounding_text (int id, int offset, int len);

public:
    virtual void init (int argc, char **argv);
    virtual void run ();

private:
//    uint32 generate_key () const;
    int generate_ctx_id ();

    bool check_client_connection (const Socket &client) const;

    void ibus_accept_callback    (SocketServer *server, const Socket &client);
    void ibus_receive_callback   (SocketServer *server, const Socket &client);
    void ibus_exception_callback (SocketServer *server, const Socket &client);

    bool ibus_open_connection    (SocketServer *server, const Socket &client);
    void ibus_close_connection   (SocketServer *server, const Socket &client);
    ClientInfo ibus_get_client_info (const Socket &client);

    //client_id is client's ibus id
    void ibus_get_factory_list            (int client_id);
    void ibus_get_factory_name            (int client_id);
    void ibus_get_factory_authors         (int client_id);
    void ibus_get_factory_credits         (int client_id);
    void ibus_get_factory_help            (int client_id);
    void ibus_get_factory_locales         (int client_id);
    void ibus_get_factory_icon_file       (int client_id);
    void ibus_get_factory_language        (int client_id);

    void ibus_new_instance                (int client_id);
    void ibus_delete_instance             (int client_id);
    void ibus_delete_all_instances        (int client_id);

    void ibus_process_key_event           (int client_id);
    void ibus_move_preedit_caret          (int client_id);
    void ibus_select_candidate            (int client_id);
    void ibus_update_lookup_table_page_size (int client_id);
    void ibus_lookup_table_page_up        (int client_id);
    void ibus_lookup_table_page_down      (int client_id);
    void ibus_reset                       (int client_id);
    void ibus_focus_in                    (int client_id);
    void ibus_focus_out                   (int client_id);
    void ibus_trigger_property            (int client_id);
    void ibus_process_helper_event        (int client_id);
    void ibus_update_client_capabilities  (int client_id);

    void ibus_flush_config                (int client_id);
    void ibus_erase_config                (int client_id);
    void ibus_get_config_string           (int client_id);
    void ibus_set_config_string           (int client_id);
    void ibus_get_config_int              (int client_id);
    void ibus_set_config_int              (int client_id);
    void ibus_get_config_bool             (int client_id);
    void ibus_set_config_bool             (int client_id);
    void ibus_get_config_double           (int client_id);
    void ibus_set_config_double           (int client_id);
    void ibus_get_config_vector_string    (int client_id);
    void ibus_set_config_vector_string    (int client_id);
    void ibus_get_config_vector_int       (int client_id);
    void ibus_set_config_vector_int       (int client_id);
    void ibus_reload_config               (int client_id);

    void ibus_load_file                   (int client_id);

    void reload_config_callback (const ConfigPointer &config);

    int portal_create_ctx                 (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int srv_destroy                       (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_focus_in                      (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_focus_out                     (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_reset                         (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_process_key_event             (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_set_cursor_location           (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_set_cursor_location_relative  (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_process_hand_writing_event    (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_cancel_hand_writing           (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_property_activate(sd_bus_message *m, sd_bus_error *ret_error);
    int ctx_set_engine                    (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_get_engine                    (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_set_surrounding_text          (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_set_capabilities              (sd_bus_message *m,
                                           sd_bus_error *ret_error);
    int ctx_get_client_commit_preedit     (sd_bus *bus,
 	                                        const char *path,
 	                                        const char *interface,
 	                                        const char *property,
 	                                        sd_bus_message *value,
 	                                        sd_bus_error *ret_error);
    int ctx_set_client_commit_preedit     (sd_bus *bus,
 	                                       const char *path,
 	                                       const char *interface,
 	                                       const char *property,
 	                                       sd_bus_message *value,
 	                                       sd_bus_error *ret_error);
    int ctx_get_content_type              (sd_bus *bus,
 	                                       const char *path,
 	                                       const char *interface,
 	                                       const char *property,
 	                                       sd_bus_message *value,
 	                                       sd_bus_error *ret_error);
    int ctx_set_content_type              (sd_bus *bus,
 	                                       const char *path,
 	                                       const char *interface,
 	                                       const char *property,
 	                                       sd_bus_message *value,
 	                                       sd_bus_error *ret_error);
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
