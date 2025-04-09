/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2022 Beaver (GEGL rock text)
 */

/*
GEGL Graph recreation of Rock Text so users can test without installing.
This may not be 100% accurate but it is close enough.

#rock text
median-blur radius=10 alpha-percentile=70
noise-spread amount-x=22 amount-y=42
gaussian-blur std-dev-y=2.5 std-dev-x=2.5
color-overlay value=#ff8100
id=1
src-atop aux=[ ref=1 layer ]
gimp:layer-mode layer-mode=multiply aux=[ ref=1 emboss depth=12 elevation=9 azimuth=4 ]
gimp:threshold-alpha
unsharp-mask scale=1.2

Yes, Rock Text and Rock Text 2 should co-exist and they are different filters.
 */


#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_enum(guichange, _("Part of filter to be displayed:"),
    guiendrockfun, guichangerockfun,
    ROCKTEXT_SHOW_DEFAULT)
  description(_("Change the GUI option"))


enum_start (guichangerockfun)
enum_value   (ROCKTEXT_SHOW_DEFAULT, "default", N_("Basic Sliders"))
enum_value   (ROCKTEXT_SHOW_LEGACY, "legacy", N_("Legacy Sliders for technical users"))
  enum_end (guiendrockfun)


property_int  (size, _("Internal Median Blur Radius"), 0)
  value_range (0, 10)
  ui_range    (0, 10)
  ui_meta     ("unit", "pixel-distance")
  description (_("Neighborhood radius, a negative value will calculate with inverted percentiles"))
ui_meta ("visible", "guichange {legacy}")

property_double  (alpha_percentile, _("Internal Median Blur Alpha percentile"), 28)
  value_range (0, 100)
  description (_("Neighborhood alpha percentile"))
ui_meta ("visible", "guichange {legacy}")

property_int    (amountx, _("Rockification Horizontal"), 33)
    description (_("Internal horizontal spread amount that makes the bevel rocky"))
    value_range (0, 112)
    ui_meta     ("unit", "pixel-distance")
    ui_meta     ("axis", "x")
  ui_range    (0.0, 70.0)

property_int    (amounty, _("Rockification Vertical"), 33)
    description (_("Internal vertical spread amount that makes the bevel rocky"))
    value_range (0, 112)
    ui_meta     ("unit", "pixel-distance")
    ui_meta     ("axis", "y")
  ui_range    (0.0, 70.0)

property_seed (seed, _("Random seed"), rand)
    description(_("Seed of the Rock Text"))


property_double (gaussian, _("Internal Gaussian Blur"), 1.8)
   description (_("Gaussian Blur to expand the rock text"))
   value_range (0.24, 4.0)
   ui_range    (0.24, 4.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")
   ui_meta     ("axis", "x")
ui_meta ("visible", "guichange {legacy}")

property_int  (shift, _("Horizontal Shift"), 1)
    description(_("Apply an internal shift filter inside the rock text"))
    value_range (0, 10)
    ui_meta    ("unit", "pixel-distance")
ui_meta ("visible", "guichange {legacy}")

property_seed (seed2, _("Random seed"), rand2)
    description(_("Another Seed of the rock text"))
ui_meta ("visible", "guichange {legacy}")

property_int  (size2, _("Crumbled mini rocks effect "), 2)
  value_range (0, 10)
  ui_range    (0, 10)
  ui_meta     ("unit", "pixel-distance")
  description (_("Crumble effect, lower values means more crumbling mini rocks. Higher means a more orderly rock."))


property_double (azimuth, _("Light Rotation"), 180.0)
    description (_("Light angle (degrees)"))
    value_range (0, 360)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")

property_double (elevation, _("Elevation"), 80.0)
    description (_("Elevation angle (degrees)"))
    value_range (0, 180)
    ui_meta ("unit", "degree")

property_double (grains, _("Grains over rock"), 0.8)
  value_range   (0.0, 1.0)


property_color  (mvalue, _("Color Overide"), "#9a7143")
    description (_("Color of the rock text"))

property_file_path(src, _("Image overlay (use white text)"), "")
    description (_("Image file overlays work proper on white colored text. Source image file paths accepted are (png, jpg, raw, svg, bmp, tif, ...)"))
ui_meta ("visible", "guichange {legacy}")

property_double (x, _("X outline"), 1.0)
  description   (_("Horizontal shadow offset"))
  ui_range      (-6.0, 6.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "x")
ui_meta ("visible", "guichange {legacy}")

property_double (y, _("Y outline"), 1.0)
  description   (_("Vertical shadow offset"))
  ui_range      (-6.0, 6.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "y")
ui_meta ("visible", "guichange {legacy}")

property_double (radius, _("Outline Blur radius"), 0.5)
  value_range   (0.0, G_MAXDOUBLE)
  ui_range      (0.0, 1.0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
ui_meta ("visible", "guichange {legacy}")

property_double (grow_radius, _("Outline Grow radius"), 0)
  value_range   (-5.0, 5.0)
  ui_range      (-5.0, 5.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The distance to expand the shadow before blurring; a negative value will contract the shadow instead"))
ui_meta ("visible", "guichange {legacy}")

property_color  (color, _("Outline Color"), "black")
    /* TRANSLATORS: the string 'black' should not be translated */
  description   (_("The shadow's color (defaults to 'black')"))
ui_meta ("visible", "guichange {legacy}")

/* It does make sense to sometimes have opacities > 1 (see GEGL logo
 * for example)
 */
property_double (opacity, _("Outline Opacity"), 0.0)
  value_range   (0.0, 1.0)
  ui_steps      (0.01, 0.10)
ui_meta ("visible", "guichange {legacy}")

property_double (exposure, _("Darkness to light"), 0.5)
    description (_("Exposure's darkness to light"))
    ui_range    (-2.0, 2.0)
ui_meta ("visible", "guichange {legacy}")



#else

#define GEGL_OP_META
#define GEGL_OP_NAME     rock_text
#define GEGL_OP_C_SOURCE rock-text.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglColor *rock_hidden_color = gegl_color_new ("#ffffff");

      GeglNode *input    = gegl_node_get_input_proxy (gegl, "input");
      GeglNode *output   = gegl_node_get_output_proxy (gegl, "output");


      GeglNode *color    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay", "value", rock_hidden_color,
                                  NULL);

      GeglNode *median    = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur",  "abyss-policy", 0,
                                  NULL);


      GeglNode *median2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur", "abyss-policy", 0,
                                  NULL);


      GeglNode *gaussian    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gaussian-blur", "clip-extent", 0, "abyss-policy", 0,
                                  NULL);

      GeglNode *noise    = gegl_node_new_child (gegl,
                                  "operation", "gegl:noise-spread",
                                  NULL);

      GeglNode *exposure    = gegl_node_new_child (gegl,
                                  "operation", "gegl:exposure",
                                  NULL);

      GeglNode *normal    = gegl_node_new_child (gegl,
                                  "operation", "gegl:over",
                                  NULL);
#define mysyntax \
" id=1 gimp:layer-mode layer-mode=hsv-value  opacity=1 aux=[  ref=1 noise-hsv holdness=7 hue-distance=0 saturation-distance=0 value-distance=0.90 ]   "\


      GeglNode *graph    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", mysyntax, 
                                  NULL);


      GeglNode *opacity    = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);




      GeglNode *edgesmooth    = gegl_node_new_child (gegl,
                                  "operation", "lb:edgesmooth", "alpha-percentile2", 67.0, "gaus", 0.75,  "value", 2.2, "abyss-policy", 0, 
                                  NULL);



      GeglNode *alpha    = gegl_node_new_child (gegl,
                                  "operation", "gimp:threshold-alpha",
                                  NULL);

      GeglNode *shift   = gegl_node_new_child (gegl,
                                  "operation", "gegl:shift",
                                  NULL);

     GeglNode *emboss    = gegl_node_new_child (gegl,
                                  "operation", "gegl:emboss",
                                  NULL);


     GeglNode *mcol    = gegl_node_new_child (gegl,
                                  "operation", "gegl:multiply",
                                  NULL);

     GeglNode *image    = gegl_node_new_child (gegl,
                                  "operation", "gegl:multiply",
                                  NULL);

     GeglNode *nop    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);

     GeglNode *smooth    = gegl_node_new_child (gegl,
                                  "operation", "gegl:mean-curvature-blur", "iterations", 1,
                                  NULL);


     GeglNode *nop2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);


     GeglNode *imagefileupload    = gegl_node_new_child (gegl,
                                  "operation", "port:load",
                                  NULL);

     GeglNode *coloroverlay    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color-overlay",
                                  NULL);

     GeglNode *outline    = gegl_node_new_child (gegl,
                                  "operation", "gegl:dropshadow",
                                  NULL);


  gegl_operation_meta_redirect (operation, "size", median, "radius");
  gegl_operation_meta_redirect (operation, "shift", shift, "shift");
  gegl_operation_meta_redirect (operation, "seed2", shift, "seed");
  gegl_operation_meta_redirect (operation, "amountx", noise, "amount-x");
  gegl_operation_meta_redirect (operation, "amounty", noise, "amount-y");
  gegl_operation_meta_redirect (operation, "seed", noise, "seed");
  gegl_operation_meta_redirect (operation, "size2", median2, "radius");
  gegl_operation_meta_redirect (operation, "gaussian", gaussian, "std-dev-x");
  gegl_operation_meta_redirect (operation, "gaussian", gaussian, "std-dev-y");
  gegl_operation_meta_redirect (operation, "azimuth", emboss, "azimuth");
  gegl_operation_meta_redirect (operation, "elevation", emboss, "elevation");
  gegl_operation_meta_redirect (operation, "alpha-percentile", median, "alpha-percentile");
  gegl_operation_meta_redirect (operation, "radius", outline, "radius");
  gegl_operation_meta_redirect (operation, "opacity", outline, "opacity");
  gegl_operation_meta_redirect (operation, "x", outline, "x");
  gegl_operation_meta_redirect (operation, "y", outline, "y");
  gegl_operation_meta_redirect (operation, "grow_radius", outline, "grow-radius");
  gegl_operation_meta_redirect (operation, "color", outline, "color");
  gegl_operation_meta_redirect (operation, "exposure", exposure, "exposure");
  gegl_operation_meta_redirect (operation, "mvalue", coloroverlay, "value");
  gegl_operation_meta_redirect (operation, "src", imagefileupload, "src");
  gegl_operation_meta_redirect (operation, "grains", opacity, "value");

  gegl_node_link_many (input, color, median, noise, gaussian, shift, median2, emboss, alpha, image, outline, nop, mcol,  nop2, normal, smooth,  exposure, edgesmooth, output, NULL);
  gegl_node_connect (image, "aux", imagefileupload, "output");
  gegl_node_connect (mcol, "aux", coloroverlay, "output");
  gegl_node_connect (normal, "aux", opacity, "output");
  gegl_node_link_many (nop2, graph, opacity, NULL);
  gegl_node_link_many (nop, coloroverlay, NULL);

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:rock-text",
    "title",       _("Rock Text"),
    "reference-hash", "30do6a1h24f10jcjf25sb2ac",
    "description", _("Make rock shaped text "
                     ""),
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Rock Text (original)..."),
    NULL);
}

#endif
