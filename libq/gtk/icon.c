
#include <libq/config.h>

#ifdef GSEAL_ENABLE
#undef GSEAL_ENABLE
#endif

#include <math.h>
#include <math.h>
#include <cairo.h>
#include <glib/gi18n.h>
#include <gtk/gtkprivate.h>
#include "icon.h"
#include "../geometry.h"


enum {
	PROP_0,
	PROP_ICON_PIXBUF,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = { NULL };

struct _QIconPrivate {
	cairo_surface_t *surface;
};

#define Q_ICON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj),Q_TYPE_ICON,QIconPrivate))


static void q_icon_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec);
static void q_icon_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec);
static void q_icon_finalize(GObject *object);
static void q_icon_size_request(GtkWidget *widget,GtkRequisition *requisition);
static void q_icon_size_allocate(GtkWidget *widget,GtkAllocation *allocation);
static gboolean q_icon_expose(GtkWidget *widget,GdkEventExpose *event);
static void q_icon_paint(QIcon *icon,const GdkRectangle *area);
static void q_icon_use_pixbuf(QIcon *icon,GdkPixbuf *pixbuf);


G_DEFINE_TYPE(QIcon,q_icon,GTK_TYPE_BUTTON)

GtkWidget *q_icon_new(GdkPixbuf *pixbuf) {
	return g_object_new(Q_TYPE_ICON,
		"icon-pixbuf",pixbuf,
		NULL);
}

static void q_icon_class_init(QIconClass *cl) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(cl);
	GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS(cl);

//fprintf(stdout,"q_icon_class_init(%p)\n",cl);

	gobject_class->set_property  = q_icon_set_property;
	gobject_class->get_property  = q_icon_get_property;
	gobject_class->finalize      = q_icon_finalize;

	widget_class->size_request   = q_icon_size_request;
	widget_class->size_allocate  = q_icon_size_allocate;
	widget_class->expose_event   = q_icon_expose;

	properties[PROP_ICON_PIXBUF] = g_param_spec_pointer("icon-pixbuf",_("Icon Pixbuf"),_("A GdkPixbuf of the icon image"),G_PARAM_WRITABLE);

	g_object_class_install_properties(gobject_class,PROP_LAST,properties);

	g_type_class_add_private(gobject_class,sizeof(QIconPrivate));
}

static void q_icon_init(QIcon *icon) {
	QIconPrivate *p = Q_ICON_GET_PRIVATE(icon);
//fprintf(stdout,"q_icon_init(%p)\n",widget);
	icon->_priv  = p;
	p->surface   = NULL;
}

static void q_icon_set_property(GObject *object,guint prop_id,const GValue *value,GParamSpec *pspec) {
	QIcon *icon = Q_ICON(object);
//fprintf(stdout,"q_icon_set_property(%p)\n",object);
	switch(prop_id) {
		case PROP_ICON_PIXBUF:
			q_icon_use_pixbuf(icon,(GdkPixbuf *)g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

static void q_icon_get_property(GObject *object,guint prop_id,GValue *value,GParamSpec *pspec) {
	switch(prop_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
			break;
	}
}

static void q_icon_finalize(GObject *object) {
	if(Q_IS_ICON(object)) {
		QIcon *icon = (QIcon *)object;
		QIconPrivate *p = icon->_priv;
		if(p->surface!=NULL) {
			cairo_surface_destroy(p->surface);
			p->surface = NULL;
		}
	}
	G_OBJECT_CLASS(q_icon_parent_class)->finalize(object);
}

static void q_icon_size_request(GtkWidget *widget,GtkRequisition *requisition) {
	if(Q_IS_ICON(widget)) {
		QIcon *icon = (QIcon *)widget;
		QIconPrivate *p = icon->_priv;
		gint w = 4,h = 4;
		if(p->surface!=NULL) {
			w += cairo_image_surface_get_width(p->surface);
			h += cairo_image_surface_get_height(p->surface);
			if(w<h) w = h;
			if(h<w) h = w;
		}
		requisition->width  = w;
		requisition->height = h;
fprintf(stdout,"q_icon_size_request(w: %d, h: %d)\n",w,h);
	}
}

static void q_icon_size_allocate(GtkWidget *widget,GtkAllocation *allocation) {
	if(Q_IS_ICON(widget)) {
		QIcon *icon = (QIcon *)widget;
		QIconPrivate *p = icon->_priv;

		GTK_WIDGET_CLASS(q_icon_parent_class)->size_allocate(widget,allocation);

		if(p->surface!=NULL) {
			gint w = cairo_image_surface_get_width(p->surface)+4;
			gint h = cairo_image_surface_get_height(p->surface)+4;
			if(h<allocation->height) h = allocation->height;
			if(w!=allocation->width || h!=allocation->height || w!=h) {
				if(w<h) w = h;
				if(h<w) h = w;
				gtk_widget_set_size_request(widget,w,h);
			}
		}
	}
}

static gboolean q_icon_expose(GtkWidget *widget,GdkEventExpose *event) {
	if(Q_IS_ICON(widget) && gtk_widget_is_drawable(widget)) {
		q_icon_paint((QIcon *)widget,&event->area);
//		if(GTK_WIDGET_CLASS(q_icon_parent_class)->expose_event)
//			return GTK_WIDGET_CLASS(q_icon_parent_class)->expose_event(widget,event);
//		GTK_WIDGET_CLASS(q_icon_parent_class)->expose_event(widget,event);
	}
	return FALSE;
}

static void q_icon_paint(QIcon *icon,const GdkRectangle *area) {
//fprintf(stdout,"q_icon_expose(%p)\n",widget);
	GtkWidget *widget = (GtkWidget *)icon;
	GtkButton *button = (GtkButton *)icon;
	QIconPrivate *p = icon->_priv;
	gint border_width = ((GtkContainer *)widget)->border_width;
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget,&alloc);
	if(alloc.width>0 && alloc.height>0) {
		gint x = alloc.x+border_width;
		gint y = alloc.y+border_width;
		gint w = alloc.width-border_width*2;
		gint h = alloc.height-border_width*2;
		GtkStateType state_type = gtk_widget_get_state(widget);
		GtkShadowType shadow_type = button->depressed? GTK_SHADOW_IN : GTK_SHADOW_OUT;
		GtkBorder default_border;
		GtkBorder default_outside_border;
		gboolean interior_focus;
		gint focus_width;
		gint focus_pad;
		gboolean displace_focus;
		gtk_widget_style_get(widget,
		                     "default-border",&default_border,
		                     "default-outside-border",&default_outside_border,
		                     "interior-focus",&interior_focus,
		                     "focus-line-width",&focus_width,
		                     "focus-padding",&focus_pad,
		                     "displace-focus",&displace_focus,NULL); 

		if(gtk_widget_has_default(widget) && button->relief==GTK_RELIEF_NORMAL) {
			gtk_paint_box(widget->style,widget->window,
			              GTK_STATE_NORMAL,GTK_SHADOW_IN,
			              area,widget,"buttondefault",
			              x,y,w,h);
			x += default_border.left;
			y += default_border.top;
			w -= default_border.left+default_border.right;
			h -= default_border.top+default_border.bottom;
		} else if(gtk_widget_get_can_default(widget)) {
			x += default_outside_border.left;
			y += default_outside_border.top;
			w -= default_outside_border.left+default_outside_border.right;
			h -= default_outside_border.top+default_outside_border.bottom;
		}

		if(gtk_widget_has_focus(widget)) {
			if(!interior_focus) {
				x += focus_width+focus_pad;
				y += focus_width+focus_pad;
				w -= (focus_width+focus_pad)*2;
				h -= (focus_width+focus_pad)*2;
			}

			if(button->depressed && displace_focus) {
				gint child_displacement_x;
				gint child_displacement_y;
				gtk_widget_style_get(widget,
				                     "child-displacement-y",&child_displacement_y,
				                     "child-displacement-x",&child_displacement_x,NULL);
				x += child_displacement_x;
				y += child_displacement_y;
			}
		}

		if(button->relief!=GTK_RELIEF_NONE || button->depressed || state_type==GTK_STATE_PRELIGHT)
			gtk_paint_box(widget->style,widget->window,
			              state_type,shadow_type,
			              area,widget,"button",
			              x,y,w,h);

		if(p->surface!=NULL) {
			cairo_t *cr = gdk_cairo_create(widget->window);
			gint iw = cairo_image_surface_get_width(p->surface);
			gint ih = cairo_image_surface_get_height(p->surface);
			gint ix = x+(w-iw)/2;
			gint iy = y+(h-ih)/2;
			cairo_translate(cr,ix,iy);
			cairo_set_source_surface(cr,p->surface,0.0,0.0);
			cairo_rectangle(cr,0.0,0.0,iw,ih);
			cairo_fill(cr);
			cairo_destroy(cr);
//fprintf(stdout,"q_icon_paint(x: %g, y: %g, w: %d, h: %d)\n",x,y,alloc.width,alloc.height);
		}

		if(gtk_widget_has_focus(widget)) {
			if(interior_focus) {
				x += widget->style->xthickness+focus_pad;
				y += widget->style->ythickness+focus_pad;
				w -= (widget->style->xthickness+focus_pad)*2;
				h -= (widget->style->ythickness+focus_pad)*2;
			} else {
				x -= focus_width+focus_pad;
				y -= focus_width+focus_pad;
				w += (focus_width+focus_pad)*2;
				h += (focus_width+focus_pad)*2;
			}
			gtk_paint_focus(widget->style,widget->window,
			                state_type,area,widget,"button",
			                x,y,w,h);
		}
	}
}

static void q_icon_use_pixbuf(QIcon *icon,GdkPixbuf *pixbuf) {
	if(pixbuf!=NULL) {
		QIconPrivate *p = icon->_priv;
		gint w = gdk_pixbuf_get_width(pixbuf);
		gint h = gdk_pixbuf_get_height(pixbuf);
		if(p->surface!=NULL)
			cairo_surface_destroy(p->surface);
		p->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,w,h);
		{
			cairo_t *cr = cairo_create(p->surface);
			gdk_cairo_set_source_pixbuf(cr,pixbuf,0.0,0.0);
			cairo_rectangle(cr,0,0,w,h);
			cairo_fill(cr);
			cairo_destroy(cr);
		}
		gtk_widget_set_size_request((GtkWidget *)icon,w+4,h+4);
		gtk_widget_queue_draw(GTK_WIDGET(icon));
//fprintf(stdout,"q_icon_use_pixbuf()\n");
	}
}