/*
 * peas-activatable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-activatable.h"
#include "peas-plugin.h"

/**
 * SECTION:peas-activatable
 * @short_description: Interface for activatable plugins
 **/

G_DEFINE_INTERFACE(PeasActivatable, peas_activatable, PEAS_TYPE_PLUGIN)

void
peas_activatable_default_init (PeasActivatableInterface *iface)
{
}

/**
 * peas_activatable_activate:
 * @activatable: A #PeasActivatable.
 * @object: The #GObject on which the plugin should be activated.
 *
 * Activates the plugin on an object.  An instance of #PeasActivatable will be
 * activated once for each object registered against the #PeasEngine which
 * controls this #PeasActivatable.  For instance, a typical GUI application
 * like gedit will activate the plugin once for each of its main windows.
 */
void
peas_activatable_activate (PeasActivatable *activatable,
                           GObject         *object)
{
  PeasActivatableInterface *iface;

  g_return_if_fail (PEAS_IS_ACTIVATABLE (activatable));
  g_return_if_fail (G_IS_OBJECT (object));

  iface = PEAS_ACTIVATABLE_GET_IFACE (activatable);
  if (iface->activate != NULL)
    iface->activate (activatable, object);
}

/**
 * peas_activatable_deactivate:
 * @activatable: A #PeasActivatable.
 * @object: A #GObject.
 *
 * Deactivates the plugin on the given object.
 */
void
peas_activatable_deactivate (PeasActivatable *activatable,
                             GObject         *object)
{
  PeasActivatableInterface *iface;

  g_return_if_fail (PEAS_IS_ACTIVATABLE (activatable));
  g_return_if_fail (G_IS_OBJECT (object));

  iface = PEAS_ACTIVATABLE_GET_IFACE (activatable);
  if (iface->deactivate != NULL)
    iface->deactivate (activatable, object);
}

/**
 * peas_activatable_update_state:
 * @activatable: A #PeasActivatable.
 * @object: A #GObject.
 *
 * Triggers an update of the plugin insternal state to take into account
 * state changes in the targetted object, due to a plugin or an user action.
 */
void
peas_activatable_update_state (PeasActivatable *activatable,
                               GObject         *object)
{
  PeasActivatableInterface *iface;

  g_return_if_fail (PEAS_IS_ACTIVATABLE (activatable));
  g_return_if_fail (G_IS_OBJECT (object));

  iface = PEAS_ACTIVATABLE_GET_IFACE (activatable);
  if (iface->update_state != NULL)
    iface->update_state (activatable, object);
}

