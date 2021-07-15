#ifndef _LIBQ_GTK_ICON_H_
#define _LIBQ_GTK_ICON_H_

/**
 * @file libq/gtk/icon.h
 * @author Per Löwgren
 * @date Modified: 2016-05-27
 * @date Created: 2016-05-27
 */

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define Q_TYPE_ICON            (q_icon_get_type())
#define Q_ICON(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),Q_TYPE_ICON,QIcon))
#define Q_ICON_CLASS(cl)       (G_TYPE_CHECK_CLASS_CAST((cl),Q_TYPE_ICON,QIconClass))
#define Q_IS_ICON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),Q_TYPE_ICON))
#define Q_IS_ICON_CLASS(cl)    (G_TYPE_CHECK_CLASS_TYPE((cl),Q_TYPE_ICON))
#define Q_ICON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),Q_TYPE_ICON,QIconClass))


typedef struct _QIcon QIcon;
typedef struct _QIconClass QIconClass;
typedef struct _QIconPrivate QIconPrivate;

struct _QIcon {
	GtkButton button;
	QIconPrivate *_priv;
};

struct _QIconClass {
	GtkButtonClass parent_class;
};

GType q_icon_get_type(void) G_GNUC_CONST;
GtkWidget *q_icon_new(GdkPixbuf *pixbuf);

G_END_DECLS

#endif /* _LIBQ_GTK_ICON_H_ */
