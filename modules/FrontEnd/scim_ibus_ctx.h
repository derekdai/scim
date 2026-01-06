/** @file scim_ibus_ctx.h
 *  @brief definition of IBusCtx class.
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
 * $Id: scim_ibus_ctx.h,v 1.25 2020/04/29 17:01:56 derekdai Exp $
 */

#ifndef __SCIM_IBUS_CTX_H
#define __SCIM_IBUS_CTX_H

#include <gio/gio.h>
#include "scim_ibus_types.h"

using namespace scim;

struct IBusContentType
{
    uint32 purpose;
    uint32 hints;

    IBusContentType ()
        : purpose (IBUS_INPUT_PURPOSE_FREE_FORM),
          hints (IBUS_INPUT_HINT_NONE) { }

    int from_variant (GVariant *v)
    {
        if (v == NULL || !g_variant_is_of_type (v, G_VARIANT_TYPE ("(uu)"))) {
            return -1;
        }
        g_variant_get (v, "(uu)", &purpose, &hints);
        return 0;
    }

    GVariant *to_variant () const
    {
        return g_variant_new ("(uu)", purpose, hints);
    }
};

struct IBusRect
{
    int32 x;
    int32 y;
    int32 w;
    int32 h;

    IBusRect () : x (0), y (0), w (0), h (0) { }

    int from_variant (GVariant *v)
    {
        if (v == NULL || !g_variant_is_of_type (v, G_VARIANT_TYPE ("(iiii)"))) {
            return -1;
        }
        g_variant_get (v, "(iiii)", &x, &y, &w, &h);
        return 0;
    }
};

class IBusCtx
{
    std::string             m_owner;
    String                  m_locale;
    String                  m_encoding;

    int                     m_id;
    int                     m_siid;

    bool                    m_on;
    bool                    m_shared_siid;

    uint32                  m_caps;
    IBusRect                m_cursor_location;
    IBusRect                m_cursor_location_relative;
    bool                    m_client_commit_preedit;
    IBusContentType         m_content_type;

    WideString              m_preedit_text;
    AttributeList           m_preedit_attrs;
    int                     m_preedit_caret;

    guint                   m_inputcontext_id;
    GDBusConnection        *m_connection;

public:
    IBusCtx (const char *owner,
             const String &locale,
             int id,
             int siid);
    ~IBusCtx ();

    int init (GDBusConnection *bus, const char *path);

    int id () const { return m_id; }
    int siid () const { return m_siid; }

    const std::string &owner () const { return m_owner; }
    const String &locale () const { return m_locale; }
    const String &encoding () const { return m_encoding; }

    bool is_on () const { return m_on; }
    void on () { m_on = true; }
    void off () { m_on = false; }

    bool shared_siid () const { return m_shared_siid; }
    void set_shared_siid (bool shared) { m_shared_siid = shared; }

    uint32 caps () const { return m_caps; }
    uint32 scim_caps () const;

    const IBusRect &cursor_location () const { return m_cursor_location; }
    const IBusRect &cursor_location_relative () const { return m_cursor_location_relative; }

    int caps_from_variant (GVariant *v);

    int content_type_from_variant (GVariant *v);
    GVariant *content_type_to_variant () const;

    int cursor_location_from_variant (GVariant *v);
    int cursor_location_relative_from_variant (GVariant *v);

    String keyboard_layout () const;

    const WideString &preedit_text () const { return m_preedit_text; }
    void preedit_text (const WideString &t) { m_preedit_text = t; }

    const AttributeList &preedit_attrs () const { return m_preedit_attrs; }
    void preedit_attrs (const AttributeList &a) { m_preedit_attrs = a; }

    int preedit_caret () const { return m_preedit_caret; }
    void preedit_caret (int c) { m_preedit_caret = c; }

    void preedit_reset () {
        m_preedit_text.clear ();
        m_preedit_attrs.clear ();
        m_preedit_caret = 0;
    }

    int client_commit_preedit_from_variant (GVariant *v);
    GVariant *client_commit_preedit_to_variant () const;
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
