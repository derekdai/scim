/**
 * @file scim_ibus_types.h
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
 * $Id: scim_ibus_types.h,v 1.56 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_TYPES_H)
#define __SCIM_IBUS_TYPES_H

class IBusPortal
{
public:
    virtual void destroy_input_context(int ic_id) = 0;
};

#endif // _SCIM_IBUS_TYPES_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
