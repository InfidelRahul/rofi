#include <glib.h>
#include <math.h>
#include "widgets/widget.h"
#include "widgets/widget-internal.h"
#include "theme.h"

/** Default padding. */
#define WIDGET_DEFAULT_PADDING    0

void widget_init ( widget *widget, const char *name )
{
    widget->name              = g_strdup ( name );
    widget->def_padding       = (Padding){ { WIDGET_DEFAULT_PADDING, PW_PX, SOLID }, { WIDGET_DEFAULT_PADDING, PW_PX, SOLID }, { WIDGET_DEFAULT_PADDING, PW_PX, SOLID }, { WIDGET_DEFAULT_PADDING, PW_PX, SOLID } };
    widget->def_border        = (Padding){ { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID } };
    widget->def_border_radius = (Padding){ { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID } };
    widget->def_margin        = (Padding){ { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID }, { 0, PW_PX, SOLID } };

    widget->padding       = rofi_theme_get_padding ( widget, "padding", widget->def_padding );
    widget->border        = rofi_theme_get_padding ( widget, "border", widget->def_border );
    widget->border_radius = rofi_theme_get_padding ( widget, "border-radius", widget->def_border_radius );
    widget->margin        = rofi_theme_get_padding ( widget, "margin", widget->def_margin );
}

void widget_set_state ( widget *widget, const char *state )
{
    if ( g_strcmp0 ( widget->state, state ) ) {
        widget->state = state;
        // Update border.
        widget->border        = rofi_theme_get_padding ( widget, "border", widget->def_border );
        widget->border_radius = rofi_theme_get_padding ( widget, "border-radius", widget->def_border_radius );

        widget_queue_redraw ( widget );
    }
}

int widget_intersect ( const widget *widget, int x, int y )
{
    if ( widget == NULL ) {
        return FALSE;
    }

    if ( x >= ( widget->x ) && x < ( widget->x + widget->w ) ) {
        if ( y >= ( widget->y ) && y < ( widget->y + widget->h ) ) {
            return TRUE;
        }
    }
    return FALSE;
}

void widget_resize ( widget *widget, short w, short h )
{
    if ( widget != NULL  ) {
        if ( widget->resize != NULL ) {
            if ( widget->w != w || widget->h != h ) {
                widget->resize ( widget, w, h );
            }
        }
        else {
            widget->w = w;
            widget->h = h;
        }
        // On a resize we always want to udpate.
        widget_queue_redraw ( widget );
    }
}
void widget_move ( widget *widget, short x, short y )
{
    if ( widget != NULL ) {
        widget->x = x;
        widget->y = y;
    }
}

gboolean widget_enabled ( widget *widget )
{
    if ( widget != NULL ) {
        return widget->enabled;
    }
    return FALSE;
}

void widget_enable ( widget *widget )
{
    if ( widget && !widget->enabled ) {
        widget->enabled = TRUE;
        widget_update ( widget );
        widget_update ( widget->parent );
        widget_queue_redraw ( widget );
    }
}
void widget_disable ( widget *widget )
{
    if ( widget && widget->enabled ) {
        widget->enabled = FALSE;
        widget_update ( widget );
        widget_update ( widget->parent );
        widget_queue_redraw ( widget );
    }
}
void widget_draw ( widget *widget, cairo_t *d )
{
    // Check if enabled and if draw is implemented.
    if ( widget && widget->enabled && widget->draw ) {
        // Don't draw if there is no space.
        if ( widget->h < 1 || widget->w < 1 ) {
            widget->need_redraw = FALSE;
            return;
        }
        // Store current state.
        cairo_save ( d );
        int  margin_left   = distance_get_pixel ( widget->margin.left, ORIENTATION_HORIZONTAL );
        int  margin_top    = distance_get_pixel ( widget->margin.top, ORIENTATION_VERTICAL );
        int  margin_right  = distance_get_pixel ( widget->margin.right, ORIENTATION_HORIZONTAL );
        int  margin_bottom = distance_get_pixel ( widget->margin.bottom, ORIENTATION_VERTICAL );
        int  radius_bl     = distance_get_pixel ( widget->border_radius.left, ORIENTATION_HORIZONTAL );
        int  radius_tr     = distance_get_pixel ( widget->border_radius.right, ORIENTATION_HORIZONTAL );
        int  radius_tl     = distance_get_pixel ( widget->border_radius.top, ORIENTATION_VERTICAL );
        int  radius_br     = distance_get_pixel ( widget->border_radius.bottom, ORIENTATION_VERTICAL );
        int  left          = distance_get_pixel ( widget->border.left, ORIENTATION_HORIZONTAL );
        int  right         = distance_get_pixel ( widget->border.right, ORIENTATION_HORIZONTAL );
        int  top           = distance_get_pixel ( widget->border.top, ORIENTATION_VERTICAL );
        int  bottom        = distance_get_pixel ( widget->border.bottom, ORIENTATION_VERTICAL );

        // Background painting.
        // Set new x/y possition.
        cairo_translate ( d, widget->x, widget->y );
        cairo_set_source_rgba ( d, 0.0, 0.0, 0.0, 1.0 );

        cairo_arc ( d, margin_left + radius_tl + left / 2.0, margin_top + radius_tl + top / 2.0, radius_tl, -1.0 * M_PI, -0.5 * M_PI );
        cairo_line_to ( d, widget->w - margin_right - radius_tr - right / 2.0, margin_top + top / 2.0 );
        cairo_arc ( d, widget->w - margin_right - radius_tr - right / 2.0, margin_top + radius_tr + top / 2.0, radius_tr, -0.5 * M_PI, 0 * M_PI );
        cairo_line_to ( d, widget->w - margin_right - right / 2.0, widget->h - margin_bottom - radius_br - bottom / 2.0 );
        cairo_arc ( d, widget->w - margin_right - radius_br - right / 2.0, widget->h - margin_bottom - radius_br - bottom / 2.0, radius_br,
                    0.0 * M_PI, 0.5 * M_PI );
        cairo_line_to ( d, margin_left + radius_bl + left / 2.0, widget->h - margin_bottom - bottom / 2.0 );
        cairo_arc ( d, margin_left + radius_bl + left / 2.0, widget->h - margin_bottom - radius_bl - bottom / 2.0, radius_bl, 0.5 * M_PI, 1.0 * M_PI );
        cairo_line_to ( d, margin_left + left / 2.0, margin_top + radius_tl + top / 2.0 );
        cairo_close_path ( d );

        cairo_set_source_rgba ( d, 1.0, 1.0, 1.0, 1.0 );
        rofi_theme_get_color ( widget, "background", d );
        cairo_fill ( d );

        if ( left || top || right || bottom ) {
            cairo_save ( d );
            cairo_new_path ( d );
            rofi_theme_get_color ( widget, "foreground", d );
            if ( left > 0 ) {
                cairo_set_line_width ( d, left );
                distance_get_linestyle ( widget->border.left, d );
                cairo_move_to ( d, margin_left + left / 2.0, margin_top + radius_tl );
                cairo_line_to ( d, margin_left + left / 2.0, widget->h - margin_bottom - radius_bl );
                cairo_stroke ( d );
            }
            if ( radius_tl > 0  ) {
                distance_get_linestyle ( widget->border.left, d );
                if ( left == top ) {
                    cairo_set_line_width ( d, left );
                    cairo_arc ( d,
                                margin_left + left / 2.0 + radius_tl, margin_top + radius_tl + top / 2.0, radius_tl, -M_PI, -0.5 * M_PI );
                    cairo_stroke ( d );
                }
                else {
                    cairo_set_line_width ( d, 0 );
                    cairo_arc ( d, margin_left + radius_tl, margin_top + radius_tl, radius_tl, -M_PI, -0.5 * M_PI );
                    double a  = ( radius_tl - left );
                    double b  = ( radius_tl - top );
                    double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_tl );
                    double a1 = atan ( a / b ) + be - 0.5 * M_PI;
                    double ai = 2 * ( 0.5 * M_PI - be );
                    cairo_arc_negative ( d,
                                         margin_left + left + cos ( a1 ) * radius_tl,
                                         margin_top + ( radius_tl ) + radius_tl * sin ( a1 ),
                                         radius_tl, -M_PI + a1 + ai, -M_PI + a1 );
                    cairo_line_to ( d, margin_left, margin_top + radius_tl );
                    cairo_fill ( d );
                }
            }
            if ( right > 0 ) {
                cairo_set_line_width ( d, right );
                distance_get_linestyle ( widget->border.right, d );
                cairo_move_to ( d, widget->w - margin_right - right / 2.0, margin_top + radius_tr );
                cairo_line_to ( d, widget->w - margin_right - right / 2.0, widget->h - margin_bottom - radius_br );
                cairo_stroke ( d );
            }
            if ( radius_tr > 0  ) {
                distance_get_linestyle ( widget->border.right, d );
                if ( top == right ) {
                    cairo_set_line_width ( d, right );
                    cairo_arc ( d, widget->w - margin_right - right / 2.0 - radius_tr, margin_top + radius_tr + right / 2.0, radius_tr, -0.5 * M_PI, 0 * M_PI );
                    cairo_stroke ( d );
                }
                else {
                    cairo_set_line_width ( d, 0 );
                    cairo_arc ( d, widget->w - margin_right - radius_tr, margin_top + radius_tr, radius_tr, -0.5 * M_PI, 0 );
                    double a  = ( radius_tr - right );
                    double b  = ( radius_tr - top );
                    double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_tr );
                    double a1 = atan ( a / b ) + be - 0.5 * M_PI;
                    double ai = 2 * ( 0.5 * M_PI - be );
                    cairo_arc_negative ( d,
                                         widget->w - margin_right - right - cos ( a1 ) * radius_tr,
                                         margin_top + radius_tr * ( 1 + sin ( a1 ) ), radius_tr,
                                         0 - a1, -a1 - ai );
                    cairo_line_to ( d, widget->w - margin_right - radius_tr, margin_top );
                    cairo_fill ( d );
                }
            }
            if ( top > 0 ) {
                cairo_set_line_width ( d, top );
                distance_get_linestyle ( widget->border.top, d );
                cairo_move_to ( d, margin_left + radius_tl, margin_top + top / 2.0 );
                cairo_line_to ( d, widget->w - margin_right - radius_tr, margin_top + top / 2.0 );
                cairo_stroke ( d );
            }
            if ( radius_bl > 0  ) {
                distance_get_linestyle ( widget->border.left, d );
                if ( bottom == left ) {
                    cairo_set_line_width ( d, left );
                    cairo_arc ( d, margin_left + left / 2.0 + radius_bl, widget->h - margin_bottom - radius_bl - left / 2.0, radius_bl, 0.5 * M_PI, 1.0 * M_PI );
                    cairo_stroke ( d );
                }
                else {
                    cairo_set_line_width ( d, 0 );
                    cairo_arc ( d, margin_left + radius_bl, widget->h - margin_bottom - radius_bl, radius_bl, 0.5 * M_PI, M_PI );
                    double a  = ( radius_bl - left );
                    double b  = ( radius_bl - bottom );
                    double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_bl );
                    double a1 = atan ( a / b ) + be - 0.5 * M_PI;
                    double ai = 2 * ( 0.5 * M_PI - be );
                    cairo_arc_negative ( d,
                                         margin_left + left + cos ( a1 ) * radius_bl,
                                         widget->h - margin_bottom - radius_bl * ( 1 + sin ( a1 ) ), radius_bl,
                                         1.0 * M_PI - a1, 1.0 * M_PI - a1 - ai );
                    cairo_line_to ( d, margin_left + radius_bl, widget->h - margin_bottom );
                    cairo_fill ( d );
                }
            }
            if ( bottom > 0 ) {
                cairo_set_line_width ( d, bottom );
                distance_get_linestyle ( widget->border.bottom, d );
                cairo_move_to ( d, margin_left + radius_bl, widget->h - bottom / 2.0 - margin_bottom );
                cairo_line_to ( d, widget->w - margin_right - radius_br, widget->h - bottom / 2.0 - margin_bottom );
                cairo_stroke ( d );
            }
            if ( radius_br > 0  ) {
                distance_get_linestyle ( widget->border.right, d );
                if ( bottom == right ) {
                    cairo_set_line_width ( d, right );
                    cairo_arc ( d, widget->w - margin_right - right / 2.0 - radius_br, widget->h - margin_bottom - radius_br - right / 2.0, radius_br, 0.0 * M_PI, 0.5 * M_PI );
                    cairo_stroke ( d );
                }
                else {
                    cairo_set_line_width ( d, 0 );
                    cairo_arc ( d, widget->w - margin_right - radius_br, widget->h - margin_bottom - radius_br, radius_br, 0.0 * M_PI, 0.5 * M_PI );
                    double a  = ( radius_br - right );
                    double b  = ( radius_br - bottom );
                    double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_br );
                    double a1 = atan ( a / b ) + be - 0.5 * M_PI;
                    double ai = 2 * ( 0.5 * M_PI - be );
                    cairo_arc_negative ( d,
                                         widget->w - margin_right - right - cos ( a1 ) * radius_br,
                                         widget->h - margin_bottom - radius_br * ( 1 + sin ( a1 ) ), radius_br,
                                         0.0 * M_PI + a1 + ai, 0.0 * M_PI + a1 );
                    cairo_line_to ( d, widget->w - margin_right, widget->h - margin_bottom - radius_br );
                    cairo_fill ( d );
                }
            }
            cairo_restore ( d );
        }
        if ( radius_tl ) {
            double a  = ( radius_tl - left );
            double b  = ( radius_tl - top );
            double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_tl );
            double a1 = atan ( a / b ) + be - 0.5 * M_PI;
            double ai = 2 * ( 0.5 * M_PI - be );
            cairo_arc ( d, margin_left + left + cos ( a1 ) * radius_tl,
                    margin_top + ( radius_tl ) + radius_tl * sin ( a1 ),
                    radius_tl, -M_PI + a1, -M_PI + a1 + ai);
        }
        cairo_line_to ( d, widget->w - margin_right - radius_tr, margin_top );
        if ( radius_tr ) {
            double a  = ( radius_tr - right );
            double b  = ( radius_tr - top );
            double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_tr );
            double a1 = atan ( a / b ) + be - 0.5 * M_PI;
            double ai = 2 * ( 0.5 * M_PI - be );
            cairo_arc ( d, widget->w - margin_right - right - cos ( a1 ) * radius_tr,
                    margin_top + radius_tr * ( 1 + sin ( a1 ) ), radius_tr,
                    -0.5*M_PI+ a1, 0.5*M_PI+a1 + ai );
        }
        cairo_line_to ( d, widget->w - margin_right, widget->h - margin_bottom - radius_br );
        if ( radius_br ) {
            double a  = ( radius_br - right );
            double b  = ( radius_br - bottom );
            double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_br );
            double a1 = atan ( a / b ) + be - 0.5 * M_PI;
            double ai = 2 * ( 0.5 * M_PI - be );
            cairo_arc ( d, widget->w - margin_right - right - cos ( a1 ) * radius_br,
                    widget->h - margin_bottom - radius_br * ( 1 + sin ( a1 ) ), radius_br,
                    0.0 * M_PI + a1 , 0.0 * M_PI + a1 + ai );
        }
        cairo_line_to ( d, margin_left + radius_bl, widget->h - margin_bottom );
        if ( radius_bl ) {
                    double a  = ( radius_bl - left );
                    double b  = ( radius_bl - bottom );
                    double be = acos ( ( 0.5 * sqrt ( a * a + b * b ) ) / radius_bl );
                    double a1 = atan ( a / b ) + be - 0.5 * M_PI;
                    double ai = 2 * ( 0.5 * M_PI - be );
                    cairo_arc ( d, margin_left + left + cos ( a1 ) * radius_bl,
                            widget->h - margin_bottom - radius_bl * ( 1 + sin ( a1 ) ), radius_bl,
                            0.5 * M_PI + a1, 0.5 * M_PI + a1 + ai );
        }
        cairo_line_to ( d, margin_left, margin_top + radius_tl );
        cairo_close_path ( d );

        cairo_clip ( d );

        widget->draw ( widget, d );
        widget->need_redraw = FALSE;

        cairo_restore ( d );
    }
}
void widget_free ( widget *wid )
{
    if ( wid ) {
        if ( wid->name ) {
            g_free ( wid->name );
        }
        if ( wid->free ) {
            wid->free ( wid );
        }
    }
}

int widget_get_height ( widget *widget )
{
    if ( widget ) {
        if ( widget->get_height ) {
            return widget->get_height ( widget );
        }
        return widget->h;
    }
    return 0;
}
int widget_get_width ( widget *widget )
{
    if ( widget ) {
        if ( widget->get_width ) {
            return widget->get_width ( widget );
        }
        return widget->w;
    }
    return 0;
}
int widget_get_x_pos ( widget *widget )
{
    if ( widget ) {
        return widget->x;
    }
    return 0;
}
int widget_get_y_pos ( widget *widget )
{
    if ( widget ) {
        return widget->y;
    }
    return 0;
}

void widget_update ( widget *widget )
{
    // When (desired )size of widget changes.
    if ( widget ) {
        if ( widget->update ) {
            widget->update ( widget );
        }
    }
}

void widget_queue_redraw ( widget *wid )
{
    if ( wid ) {
        widget *iter = wid;
        // Find toplevel widget.
        while ( iter->parent != NULL ) {
            iter->need_redraw = TRUE;
            iter              = iter->parent;
        }
        iter->need_redraw = TRUE;
    }
}

gboolean widget_need_redraw ( widget *wid )
{
    if ( wid && wid->enabled ) {
        return wid->need_redraw;
    }
    return FALSE;
}
gboolean widget_clicked ( widget *wid, xcb_button_press_event_t *xbe )
{
    if ( wid && wid->clicked ) {
        return wid->clicked ( wid, xbe, wid->clicked_cb_data );
    }
    return FALSE;
}
void widget_set_clicked_handler ( widget *wid, widget_clicked_cb cb, void *udata )
{
    if ( wid ) {
        wid->clicked         = cb;
        wid->clicked_cb_data = udata;
    }
}

gboolean widget_motion_notify ( widget *wid, xcb_motion_notify_event_t *xme )
{
    if ( wid && wid->motion_notify ) {
        wid->motion_notify ( wid, xme );
    }

    return FALSE;
}

int widget_padding_get_left ( const widget *wid )
{
    int distance = distance_get_pixel ( wid->padding.left, ORIENTATION_HORIZONTAL );
    distance += distance_get_pixel ( wid->border.left, ORIENTATION_HORIZONTAL );
    distance += distance_get_pixel ( wid->margin.left, ORIENTATION_HORIZONTAL );
    return distance;
}
int widget_padding_get_right ( const widget *wid )
{
    int distance = distance_get_pixel ( wid->padding.right, ORIENTATION_HORIZONTAL );
    distance += distance_get_pixel ( wid->border.right, ORIENTATION_HORIZONTAL );
    distance += distance_get_pixel ( wid->margin.right, ORIENTATION_HORIZONTAL );
    return distance;
}
int widget_padding_get_top ( const widget *wid )
{
    int distance = distance_get_pixel ( wid->padding.top, ORIENTATION_VERTICAL );
    distance += distance_get_pixel ( wid->border.top, ORIENTATION_VERTICAL );
    distance += distance_get_pixel ( wid->margin.top, ORIENTATION_VERTICAL );
    return distance;
}
int widget_padding_get_bottom ( const widget *wid )
{
    int distance = distance_get_pixel ( wid->padding.bottom, ORIENTATION_VERTICAL );
    distance += distance_get_pixel ( wid->border.bottom, ORIENTATION_VERTICAL );
    distance += distance_get_pixel ( wid->margin.bottom, ORIENTATION_VERTICAL );
    return distance;
}

int widget_padding_get_remaining_width ( const widget *wid )
{
    int width = wid->w;
    width -= widget_padding_get_left ( wid );
    width -= widget_padding_get_right ( wid );
    return width;
}
int widget_padding_get_remaining_height ( const widget *wid )
{
    int height = wid->h;
    height -= widget_padding_get_top ( wid );
    height -= widget_padding_get_bottom ( wid );
    return height;
}
int widget_padding_get_padding_height ( const widget *wid )
{
    int height = 0;
    height += widget_padding_get_top ( wid );
    height += widget_padding_get_bottom ( wid );
    return height;
}
int widget_padding_get_padding_width ( const widget *wid )
{
    int width = 0;
    width += widget_padding_get_left ( wid );
    width += widget_padding_get_right ( wid );
    return width;
}

int widget_get_desired_height ( widget *wid )
{
    if ( wid && wid->get_desired_height ) {
        return wid->get_desired_height ( wid );
    }
    return 0;
}
