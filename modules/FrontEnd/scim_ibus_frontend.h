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
 * $Id: scim_ibus_frontend.h,v 1.26 2020/04/29 17:01:56 derekdai Exp $
 */

#if !defined (__SCIM_IBUS_FRONTEND_H)
#define __SCIM_IBUS_FRONTEND_H

#include "scim_stl_map.h"
#include <gio/gio.h>

using namespace scim;

class IBusCtx;

class IBusFrontEnd : public FrontEndBase
{
    friend class IBusCtx;

    typedef std::map <int, IBusCtx*>                                        IBusIDCtxMap;
    typedef std::set <IBusCtx*>                                             IBusCtxSet;
    typedef std::map <std::string, IBusCtxSet>                              IBusNameCtxMap;

    ConfigPointer           m_config;

    FrontEndHotkeyMatcher   m_frontend_hotkey_matcher;
    IMEngineHotkeyMatcher   m_imengine_hotkey_matcher;

    // siid
    int                     m_current_instance;

    IBusCtx                *m_current_ibus_ctx;

    PanelClient             m_panel_client;

    int                     m_ctx_counter;

    IBusIDCtxMap            m_id_ctx_map;
    IBusIDCtxMap            m_siid_ctx_map;
    IBusNameCtxMap          m_name_ctx_map;

    GMainLoop              *m_loop;
    GDBusConnection        *m_bus;
    guint                   m_portal_id;
    guint                   m_match_id;
    guint                   m_name_owner_id;
    guint                   m_panel_source;

public:
    IBusFrontEnd (const BackEndPointer &backend,
                    const ConfigPointer  &config);

    virtual ~IBusFrontEnd ();

protected:
    /**
     * vitual functions to notify client something happened,
     * see scim_frontend.cpp for explaination
     */
    virtual void show_preedit_string     (int siid);
    virtual void show_aux_string         (int siid);
    virtual void show_lookup_table       (int siid);

    virtual void hide_preedit_string     (int siid);
    virtual void hide_aux_string         (int siid);
    virtual void hide_lookup_table       (int siid);

    virtual void update_preedit_caret    (int siid, int caret);
    virtual void update_preedit_string   (int siid, const WideString & str, const AttributeList & attrs);
    virtual void update_aux_string       (int siid, const WideString & str, const AttributeList & attrs);
    virtual void commit_string           (int siid, const WideString & str);
    virtual void forward_key_event       (int siid, const KeyEvent & key);
    virtual void update_lookup_table     (int siid, const LookupTable & table);

    virtual void register_properties     (int siid, const PropertyList & properties);
    virtual void update_property         (int siid, const Property & property);

    virtual void beep                    (int siid);

    virtual void start_helper            (int siid, const String &helper_uuid);
    virtual void stop_helper             (int siid, const String &helper_uuid);
    virtual void send_helper_event       (int siid, const String &helper_uuid, const Transaction &trans);

    virtual bool get_surrounding_text    (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after);
    virtual bool delete_surrounding_text (int siid, int offset, int len);

public:
    virtual void init (int argc, char **argv);
    virtual void run ();

    // GDBus Callbacks
    static void on_bus_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void on_name_acquired (GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void on_name_lost (GDBusConnection *connection, const gchar *name, gpointer user_data);

    // GDBus Portal Interface VTable
    static void portal_method_call (GDBusConnection       *connection,
                                    const gchar           *sender,
                                    const gchar           *object_path,
                                    const gchar           *interface_name,
                                    const gchar           *method_name,
                                    GVariant              *parameters,
                                    GDBusMethodInvocation *invocation,
                                    gpointer               user_data);

    // GDBus InputContext Interface VTable
    static void input_context_method_call (GDBusConnection       *connection,
                                           const gchar           *sender,
                                           const gchar           *object_path,
                                           const gchar           *interface_name,
                                           const gchar           *method_name,
                                           GVariant              *parameters,
                                           GDBusMethodInvocation *invocation,
                                           gpointer               user_data);

    static GVariant *input_context_get_property (GDBusConnection       *connection,
                                                 const gchar           *sender,
                                                 const gchar           *object_path,
                                                 const gchar           *interface_name,
                                                 const gchar           *property_name,
                                                 GError               **error,
                                                 gpointer               user_data);

    static gboolean input_context_set_property (GDBusConnection       *connection,
                                                const gchar           *sender,
                                                const gchar           *object_path,
                                                const gchar           *interface_name,
                                                const gchar           *property_name,
                                                GVariant              *value,
                                                GError               **error,
                                                gpointer               user_data);

private:
    int generate_ctx_id ();
    bool is_current_ctx                   (const IBusCtx *ctx) const { return ctx == m_current_ibus_ctx; }

    bool check_client_connection          (const Socket &client) const;

    void reload_config_callback (const ConfigPointer &config);

    // IBus methods (internal)
    void portal_create_ctx                 (GDBusMethodInvocation *invocation, GVariant *parameters);

    // Internal handlers for context methods
    void ctx_focus_in                      (IBusCtx *ctx, GDBusMethodInvocation *invocation);
    void ctx_focus_out                     (IBusCtx *ctx, GDBusMethodInvocation *invocation);
    void ctx_reset                         (IBusCtx *ctx, GDBusMethodInvocation *invocation);
    void ctx_process_key_event             (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_set_cursor_location           (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_set_cursor_location_relative  (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_process_hand_writing_event    (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_cancel_hand_writing           (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_property_activate             (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_set_engine                    (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_get_engine                    (IBusCtx *ctx, GDBusMethodInvocation *invocation);
    void ctx_set_surrounding_text          (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);
    void ctx_set_capabilities              (IBusCtx *ctx, GDBusMethodInvocation *invocation, GVariant *parameters);

    // Property handlers
    GVariant* ctx_get_client_commit_preedit (IBusCtx *ctx);
    gboolean  ctx_set_client_commit_preedit (IBusCtx *ctx, GVariant *value);
    GVariant* ctx_get_content_type          (IBusCtx *ctx);
    gboolean  ctx_set_content_type          (IBusCtx *ctx, GVariant *value);

    // Signal handler for NameOwnerChanged
    static void on_name_owner_changed (GDBusConnection *connection,
                                       const gchar     *sender_name,
                                       const gchar     *object_path,
                                       const gchar     *interface_name,
                                       const gchar     *signal_name,
                                       GVariant        *parameters,
                                       gpointer         user_data);

    void client_destroy                   (const char *name);

    IBusCtx *find_ctx_by_siid             (int siid) const;
    IBusCtx *find_ctx                     (int id) const;
    IBusCtx *find_ctx                     (const char *path) const;

    inline bool validate_ctx              (IBusCtx *ctx) const;

    void start_ctx                        (IBusCtx *ctx);
    void stop_ctx                         (IBusCtx *ctx);
    void signal_ctx                       (int siid,
                                           const char *signal,
                                           ...) const;

    void destroy_ctx                      (IBusCtx *ctx);

    int panel_connect                     ();
    void panel_disconnect                 ();
    static gboolean panel_io_func         (GIOChannel *source,
                                           GIOCondition condition,
                                           gpointer data);

    void panel_slot_reload_config (int client_id);
    void panel_slot_exit          (int client_id);
    void panel_slot_update_lookup_table_page_size (int client_id, int page_size);
    void panel_slot_lookup_table_page_up (int client_id);
    void panel_slot_lookup_table_page_down (int client_id);
    void panel_slot_trigger_property (int client_id, const String &property);
    void panel_slot_process_helper_event (int client_id, const String &target_uuid, const String &helper_uuid, const Transaction &trans);
    void panel_slot_move_preedit_caret (int client_id, int caret_pos);
    void panel_slot_select_candidate (int client_id, int cand_index);
    void panel_slot_process_key_event (int client_id, const KeyEvent &key);
    void panel_slot_commit_string (int client_id, const WideString &wstr);
    void panel_slot_forward_key_event (int client_id, const KeyEvent &key);
    void panel_slot_request_help (int client_id);
    void panel_slot_request_factory_menu (int client_id);
    void panel_slot_change_factory (int client_id, const String &uuid);

    void panel_req_update_screen          (const IBusCtx *ctx);
    void panel_req_show_help              (const IBusCtx *ctx);
    void panel_req_show_factory_menu      (const IBusCtx *ctx);
    void panel_req_focus_in               (const IBusCtx *ctx);
    void panel_req_focus_out              (const IBusCtx *ctx);
    void panel_req_update_factory_info    (const IBusCtx *ctx);
    void panel_req_update_spot_location   (const IBusCtx *ctx);

    bool filter_hotkeys                   (IBusCtx *ctx, const KeyEvent &key);
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
