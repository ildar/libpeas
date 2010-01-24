/*
 * peas-object-module.c
 * This file is part of libpeas
 *
 * Copyright (C) 2003 Marco Pesenti Gritti
 * Copyright (C) 2003-2004 Christian Persch
 * Copyright (C) 2005-2007 Paolo Maggi
 * Copyright (C) 2008 Jesse van den Kieboom
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

#include "config.h"

#include <string.h>

#include "peas-object-module.h"

G_DEFINE_TYPE (PeasObjectModule, peas_object_module, G_TYPE_TYPE_MODULE);

typedef GObject *(*PeasObjectModuleRegisterFunc) (GTypeModule *);

enum {
  PROP_0,
  PROP_MODULE_NAME,
  PROP_PATH,
  PROP_TYPE_REGISTRATION,
  PROP_RESIDENT
};

struct _PeasObjectModulePrivate {
  GModule *library;

  PeasObjectModuleRegisterFunc register_func;

  gchar *path;
  gchar *module_name;
  gchar *type_registration;

  gboolean resident;
};

static gboolean
peas_object_module_load (GTypeModule *gmodule)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (gmodule);
  gchar *path;

  path = g_module_build_path (module->priv->path, module->priv->module_name);
  g_return_val_if_fail (path != NULL, FALSE);

  /* g_module_build_path() will add G_MODULE_SUFFIX to the path, but otoh
   * g_module_open() will only try to load the libtool archive if there is no
   * suffix specified. So we remove G_MODULE_SUFFIX here (this allows
   * uninstalled builds to load plugins as well, as there is only the .la file
   * in the build directory) */
  if (G_MODULE_SUFFIX[0] != '\0' && g_str_has_suffix (path, "." G_MODULE_SUFFIX))
    path[strlen (path) - strlen (G_MODULE_SUFFIX) - 1] = '\0';

  module->priv->library = g_module_open (path, G_MODULE_BIND_LAZY);
  g_free (path);

  if (module->priv->library == NULL)
    {
      g_warning ("%s: %s", module->priv->module_name, g_module_error ());

      return FALSE;
    }

  /* extract symbols from the lib */
  if (!g_module_symbol (module->priv->library,
                        module->priv->type_registration,
                        (void *) &module->priv->register_func))
    {
      g_warning ("%s: %s", module->priv->module_name, g_module_error ());
      g_module_close (module->priv->library);

      return FALSE;
    }

  /* symbol can still be NULL even though g_module_symbol
   * returned TRUE */
  if (module->priv->register_func == NULL)
    {
      g_warning ("Symbol '%s' should not be NULL",
                 module->priv->type_registration);
      g_module_close (module->priv->library);

      return FALSE;
    }

  if (module->priv->resident)
    g_module_make_resident (module->priv->library);

  return TRUE;
}

static void
peas_object_module_unload (GTypeModule *gmodule)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (gmodule);

  g_module_close (module->priv->library);

  module->priv->library = NULL;
  module->priv->register_func = NULL;
}

static void
peas_object_module_init (PeasObjectModule *module)
{
  module->priv = G_TYPE_INSTANCE_GET_PRIVATE (module,
                                              PEAS_TYPE_OBJECT_MODULE,
                                              PeasObjectModulePrivate);
}

static void
peas_object_module_finalize (GObject *object)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);

  g_free (module->priv->path);
  g_free (module->priv->module_name);
  g_free (module->priv->type_registration);

  G_OBJECT_CLASS (peas_object_module_parent_class)->finalize (object);
}

static void
peas_object_module_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      g_value_set_string (value, module->priv->module_name);
      break;
    case PROP_PATH:
      g_value_set_string (value, module->priv->path);
      break;
    case PROP_TYPE_REGISTRATION:
      g_value_set_string (value, module->priv->type_registration);
      break;
    case PROP_RESIDENT:
      g_value_set_boolean (value, module->priv->resident);
      break;
    default:
      g_return_if_reached ();
    }
}

static void
peas_object_module_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasObjectModule *module = PEAS_OBJECT_MODULE (object);

  switch (prop_id)
    {
    case PROP_MODULE_NAME:
      module->priv->module_name = g_value_dup_string (value);
      g_type_module_set_name (G_TYPE_MODULE (object),
                              module->priv->module_name);
      break;
    case PROP_PATH:
      module->priv->path = g_value_dup_string (value);
      break;
    case PROP_TYPE_REGISTRATION:
      module->priv->type_registration = g_value_dup_string (value);
      break;
    case PROP_RESIDENT:
      module->priv->resident = g_value_get_boolean (value);
      break;
    default:
      g_return_if_reached ();
    }
}

static void
peas_object_module_class_init (PeasObjectModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

  object_class->set_property = peas_object_module_set_property;
  object_class->get_property = peas_object_module_get_property;
  object_class->finalize = peas_object_module_finalize;

  module_class->load = peas_object_module_load;
  module_class->unload = peas_object_module_unload;

  g_object_class_install_property (object_class,
                                   PROP_MODULE_NAME,
                                   g_param_spec_string ("module-name",
                                                        "Module Name",
                                                        "The module to load for this object",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_PATH,
                                   g_param_spec_string ("path",
                                                        "Path",
                                                        "The path to use when loading this module",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_TYPE_REGISTRATION,
                                   g_param_spec_string ("type-registration",
                                                        "Type Registration",
                                                        "The name of the type registration function",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_RESIDENT,
                                   g_param_spec_boolean ("resident",
                                                         "Resident",
                                                         "Whether the module is resident",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PeasObjectModulePrivate));
}

PeasObjectModule *
peas_object_module_new (const gchar *module_name,
                        const gchar *path,
                        const gchar *type_registration,
                        gboolean     resident)
{
  return (PeasObjectModule *) g_object_new (PEAS_TYPE_OBJECT_MODULE,
                                            "module-name", module_name,
                                            "path", path,
                                            "type-registration", type_registration,
                                            "resident", resident,
                                            NULL);
}

GObject *
peas_object_module_new_object (PeasObjectModule *module)
{
  g_return_val_if_fail (module->priv->register_func != NULL, NULL);

  return module->priv->register_func (G_TYPE_MODULE (module));
}

const gchar *
peas_object_module_get_path (PeasObjectModule *module)
{
  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return module->priv->path;
}

const gchar *
peas_object_module_get_module_name (PeasObjectModule *module)
{
  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return module->priv->module_name;
}

const gchar *
peas_object_module_get_type_registration (PeasObjectModule *module)
{
  g_return_val_if_fail (PEAS_IS_OBJECT_MODULE (module), NULL);

  return module->priv->type_registration;
}
