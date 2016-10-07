#include "internal.h"
#include "plugin.h"
#include "notify.h"
#include "version.h"

#include "gtkft.h"
#include "prefs.h"
#include "stock.h"
#include "gtkcellrendererprogress.h"

GaimPlugin *plugin_handle = NULL;
static GaimGtkXferDialog *ft_dialog;

struct _GaimGtkXferDialog
{
	gboolean keep_open;
	gboolean auto_clear;

	gint num_transfers;

	GaimXfer *selected_xfer;

	GtkWidget *window;
	GtkWidget *tree;
	GtkListStore *model;

	GtkWidget *disclosure;

	GtkWidget *table;

	GtkWidget *local_user_desc_label;
	GtkWidget *local_user_label;
	GtkWidget *remote_user_desc_label;
	GtkWidget *remote_user_label;
	GtkWidget *protocol_label;
	GtkWidget *filename_label;
	GtkWidget *localfile_label;
	GtkWidget *status_label;
	GtkWidget *speed_label;
	GtkWidget *time_elapsed_label;
	GtkWidget *time_remaining_label;

	GtkWidget *progress;

	/* Buttons */
	GtkWidget *open_button;
	GtkWidget *pause_button;
	GtkWidget *resume_button;
	GtkWidget *remove_button;
	GtkWidget *stop_button;
	GtkWidget *close_button;
};

/*******************************************************************
 * GtkPieChart *****************************************************
 *******************************************************************/
#define GTK_TYPE_PIE_CHART         (gtk_pie_chart_get_type())
#define GTK_PIE_CHART(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_PIE_CHART, GtkPieChart))
#define GTK_PIE_CHART_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PIE_CHART, GtkPieChart))
#define GTK_IS_PIE_CHART(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PIE_CHART))
#define GTK_IS_PIE_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PIE_CHART))
#define GTK_PIE_CHART_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PIE_CHART, GtkPieChart))

typedef struct _GtkPieChart GtkPieChart;
typedef struct _GtkPieChartClass GtkPieChartClass;

struct _GtkPieChart {
	GtkProgressBar parent;
};

struct _GtkPieChartClass {
	GtkProgressBarClass parent_class;
};

static void
gtk_pie_chart_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->height = 20;
	requisition->width = 20;
}

static gint
gtk_pie_chart_expose (GtkWidget *widget, GdkEventExpose *event)
{
	double progress;
	GdkGC *gc = gdk_gc_new(widget->window);
	
	gdk_gc_set_rgb_fg_color(gc, &widget->style->base[GTK_STATE_NORMAL]);
	gdk_draw_arc(widget->window, gc, TRUE,
	0, 0, widget->allocation.width, widget->allocation.height,
			0, 64*360);
	
	g_object_get(widget, "fraction", &progress, NULL);
	gdk_gc_set_rgb_fg_color(gc, &widget->style->bg[GTK_STATE_SELECTED]);	
	gdk_draw_arc(widget->window, gc, TRUE,
	0, 0, widget->allocation.width, widget->allocation.height,
			64*90, (64*360)*progress);


	gdk_gc_set_rgb_fg_color(gc, &widget->style->fg[GTK_STATE_NORMAL]);
	gdk_draw_arc(widget->window, gc, FALSE,
	0, 0, widget->allocation.width, widget->allocation.height,
			0, 64*360);
	return FALSE;
}

static void gtk_pie_chart_class_init (GtkPieChartClass *class){
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	
	widget_class->size_request = gtk_pie_chart_size_request;
	widget_class->expose_event = gtk_pie_chart_expose;	
}

GType  gtk_pie_chart_get_type (void)
{
	static GType pie_chart_type = 0;
	
	if (!pie_chart_type)
		{
			static const GTypeInfo pie_chart_info =
				{
					sizeof (GtkPieChartClass),
					NULL,           /* base_init */
					NULL,           /* base_finalize */
					(GClassInitFunc) gtk_pie_chart_class_init,
					NULL,           /* class_finalize */
					NULL,           /* class_data */
					sizeof (GtkPieChart),
					0,              /* n_preallocs */
					NULL            /* instance_init */
				};
			
			pie_chart_type =
				g_type_register_static (GTK_TYPE_PROGRESS_BAR,
										"GtkPieChart",
										&pie_chart_info, 0);
		}
	
	return pie_chart_type;
}


GtkWidget *gtk_pie_chart_new(void)
{
	return g_object_new(GTK_TYPE_PIE_CHART, NULL);
}


/********************************************************************
 * GtkCellRendererTransfer ******************************************
 ********************************************************************/
#define GTK_TYPE_CELL_RENDERER_TRANSFER         (gtk_cell_renderer_transfer_get_type())
#define GTK_CELL_RENDERER_TRANSFER(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_CELL_RENDERER_TRANSFER, GtkCellRendererTransfer))
#define GTK_CELL_RENDERER_TRANSFER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CELL_RENDERER_TRANSFER, GtkCellTransferClass))
#define GTK_IS_CELL_RENDERER_TRANSFER(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_GTK_CELL_RENDERER_TRANSFER))
#define GTK_IS_CELL_RENDERER_TRANSFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GTK_CELL_RENDERER_TRANSFER))
#define GTK_CELL_RENDERER_TRANSFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CELL_RENDERER_TRANSFER, GtkCellRendererTransferClass))

typedef struct _GtkCellRendererTransfer GtkCellRendererTransfer;
typedef struct _GtkCellRendererTransferClass GtkCellRendererTransferClass;

struct _GtkCellRendererTransfer {
	GtkCellRenderer parent;

	GdkPixbuf *icon;
	gdouble progress;
	gchar *name;
	gchar *size;
	gchar *remaining;
};

struct _GtkCellRendererTransferClass {
	GtkCellRendererClass parent_class;
};

enum {
	PROP_0,
	PROP_ICON,
	PROP_NAME,
	PROP_PROGRESS,
	PROP_SIZE,
	PROP_REMAINING
};


static void gtk_cell_renderer_transfer_class_init (GtkCellRendererTransferClass *class);
GType  gtk_cell_renderer_transfer_get_type (void)
{
	static GType cell_transfer_type = 0;
	
	if (!cell_transfer_type)
		{
			static const GTypeInfo cell_transfer_info =
				{
					sizeof (GtkCellRendererTransferClass),
					NULL,           /* base_init */
					NULL,           /* base_finalize */
					(GClassInitFunc) gtk_cell_renderer_transfer_class_init,
					NULL,           /* class_finalize */
					NULL,           /* class_data */
					sizeof (GtkCellRendererTransfer),
					0,              /* n_preallocs */
					NULL            /* instance_init */
				};
			
			cell_transfer_type =
				g_type_register_static (GTK_TYPE_CELL_RENDERER,
										"GtkCellRendererTransfer",
										&cell_transfer_info, 0);
		}
	
	return cell_transfer_type;
}

static void gtk_cell_renderer_transfer_get_property (GObject    *object,
						     guint      param_id,
						     GValue     *value,
						     GParamSpec *psec)
{
	GtkCellRendererTransfer *cellrenderer = GTK_CELL_RENDERER_TRANSFER(object);

	switch (param_id)
		{
		case PROP_ICON:
			g_value_set_object(value, cellrenderer->icon);
			break;
		case PROP_PROGRESS:
			g_value_set_double(value, cellrenderer->progress);
			break;
		case PROP_NAME:
			g_value_set_string(value, cellrenderer->name);
			break;
		case PROP_SIZE:
			g_value_set_string(value, cellrenderer->size);
			break;
		case PROP_REMAINING:
			g_value_set_string(value, cellrenderer->remaining);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, psec);
			break;
		}
}

static void gtk_cell_renderer_transfer_set_property (GObject      *object,
						     guint        param_id,
						     const GValue *value,
						     GParamSpec   *pspec)
{
	GtkCellRendererTransfer *cellrenderer = GTK_CELL_RENDERER_TRANSFER (object);

	switch (param_id)
		{
		case PROP_ICON:
			if (cellrenderer->icon)
				g_object_unref(cellrenderer->icon);
			cellrenderer->icon = g_value_get_object(value);
			g_object_ref(cellrenderer->icon);
			break;
		case PROP_PROGRESS:
			cellrenderer->progress = g_value_get_double(value);
		case PROP_NAME:
			if (cellrenderer->name)
				g_free(cellrenderer->name);
			cellrenderer->name = g_strdup(g_value_get_string(value));
			break;
		case PROP_SIZE:
			if (cellrenderer->size)
				g_free(cellrenderer->size);
			cellrenderer->size = g_strdup(g_value_get_string(value));
			break;
		case PROP_REMAINING:
			if (cellrenderer->remaining)
				g_free(cellrenderer->remaining);
			cellrenderer->remaining = g_strdup(g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
			break;
		}
}


static void gtk_cell_renderer_transfer_get_size (GtkCellRenderer *cell,
						 GtkWidget       *widget,
						 GdkRectangle    *cell_area,
						 gint            *x_offset,
						 gint            *y_offset,
						 gint            *width,
						 gint            *height)
{
	GtkCellRendererTransfer *celltransfer = (GtkCellRendererTransfer *) cell;
	PangoLayout *layout = gtk_widget_create_pango_layout(widget, NULL);
	char *markup;
	
	gint calc_width = 0;
	gint calc_height = 0;

	gint icon_width;
	gint icon_height;
	
	gint string_width;
	gint string_height;

	/* Text Size */
	markup = g_strdup_printf("<b>%s</b> (%s) - <span size=\"smaller\" color=\"gray\">%s Remaining</span>",
                                  celltransfer->name, celltransfer->size, celltransfer->remaining);
	pango_layout_set_markup(layout, markup, strlen(markup));
	pango_layout_get_pixel_size (layout, &string_width, &string_height);
	g_free(markup);
	calc_width += string_width + 2;
	calc_height += string_height + 2;

	/* Progress Bar */
	calc_height += 7 + 2;

	/* Icon Size */
	if (celltransfer->icon) {
		icon_width = gdk_pixbuf_get_width (celltransfer->icon);
		icon_height = gdk_pixbuf_get_height (celltransfer->icon);
		calc_width += icon_width;
		calc_height = MAX(calc_height, icon_height);
	}

	/* padding */	
	calc_width += (gint) cell->xpad * 2;
	calc_height += (gint) cell->ypad * 2;
	

	if (width)
		*width = calc_width;
	
	if (height)
		*height = calc_height;
	
	if (cell_area)
		{
			if (x_offset)
				{
					*x_offset = cell->xalign * (cell_area->width - calc_width);
					*x_offset = MAX (*x_offset, 0);
				}
			if (y_offset)
				{
					*y_offset = cell->yalign * (cell_area->height - calc_height);
					*y_offset = MAX (*y_offset, 0);
				}
		}
}


static void gtk_cell_renderer_transfer_render (GtkCellRenderer *cell,
					       GdkWindow       *window,
					       GtkWidget       *widget,
					       GdkRectangle    *background_area,
					       GdkRectangle    *cell_area,
					       GdkRectangle    *expose_area,
					       guint            flags)
{
	GtkCellRendererTransfer *celltransfer = (GtkCellRendererTransfer *) cell;
	GdkGC *gc = gdk_gc_new(window);
	PangoLayout *layout = gtk_widget_create_pango_layout(widget, NULL);;
	GtkStyle *style = widget->style;
	gchar *markup;
	gint width, height, icon_width, string_height;
	
	width = cell_area->width;
	height = cell_area->height;
	
	/* Icon */
	icon_width = gdk_pixbuf_get_width (celltransfer->icon) + 2;
	gdk_pixbuf_render_to_drawable(celltransfer->icon, GDK_DRAWABLE(window), NULL, 
                    	0, 0, 
			cell_area->x + cell->xpad, 
			cell_area->y + cell->ypad, 	
			-1, -1, GDK_RGB_DITHER_NONE, 0, 0);

	/* Text */
	markup = g_strdup_printf("<b>%s</b> (%s) - <span size=\"smaller\" color=\"gray\">%s Remaining</span>",
                                  celltransfer->name, celltransfer->size, celltransfer->remaining);
	pango_layout_set_markup(layout, markup, strlen(markup));
	pango_layout_get_pixel_size (layout, NULL, &string_height);
	gtk_paint_layout (style, window, GTK_STATE_NORMAL, FALSE,
			  NULL, widget, NULL, cell_area->x + cell->xpad + icon_width + 2,
                                              cell_area->y + cell->ypad, layout);
	g_free(markup);

	gdk_gc_set_rgb_fg_color(gc, &widget->style->fg[GTK_STATE_NORMAL]);
	gdk_draw_rectangle (window, gc, TRUE,
		       cell_area->x + cell->xpad + icon_width + 2,
		       cell_area->y + cell->ypad + string_height + 2,
		       width - icon_width - 2, 7);
	
	gdk_gc_set_rgb_fg_color(gc, &widget->style->bg[GTK_STATE_NORMAL]);
	gdk_draw_rectangle (window, gc, TRUE,
		       cell_area->x + cell->xpad + icon_width + 3,
		       cell_area->y + cell->ypad + string_height + 3,
		       width - icon_width - 4, 5);

	gdk_gc_set_rgb_fg_color(gc, &widget->style->bg[GTK_STATE_SELECTED]);
	gdk_draw_rectangle (window, gc, TRUE,
		       cell_area->x + cell->xpad + icon_width + 3,
		       cell_area->y + cell->ypad + string_height + 3,
		       (width - icon_width - 4) * celltransfer->progress,
		       5);
}

static void gtk_cell_renderer_transfer_class_init (GtkCellRendererTransferClass *class){

	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(class);
	
	object_class->get_property = gtk_cell_renderer_transfer_get_property;
	object_class->set_property = gtk_cell_renderer_transfer_set_property;
	
	cell_class->get_size = gtk_cell_renderer_transfer_get_size;
	cell_class->render   = gtk_cell_renderer_transfer_render;

	g_object_class_install_property (object_class,
				   PROP_ICON,
				   g_param_spec_object ("pixbuf",
							"Pixbuf Object",
							"The pixbuf to render",
							GDK_TYPE_PIXBUF,
							G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_PROGRESS,
					 g_param_spec_double ("progress",
							      "Progress",
							      "The fractional progress to display",
							      0, 1, 0,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
							      "Name",
							      "File name",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_SIZE,
					 g_param_spec_string ("size",
							      "Size",
							      "Text to show as total size",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_REMAINING,
					g_param_spec_string("remaining",
							    "Remaining",
							    "Text representing remaining time for completeion",
							    NULL,
							    G_PARAM_READWRITE));
}



GtkCellRenderer *gtk_cell_renderer_transfer_new(void)
{
	return g_object_new(GTK_TYPE_CELL_RENDERER_TRANSFER, NULL);
}


/********************************************************************
 * GtkFileTransferDialog ********************************************
 ********************************************************************/ 

/* #definitions and data types */
#define GTK_TYPE_FILE_TRANSFER_DIALOG            (gtk_file_transfer_dialog_get_type ())
#define GTK_FILE_TRANSFER_DIALOG(obj)            (GTK_CHECK_CAST ((obj), GTK_TYPE_FILE_TRANSFER_DIALOG, GtkFileTransferDialog))
#define GTK_FILE_TRANSFER_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FILE_TRANSFER_DIALOG, \
                                                   GtkFileTransferDialogClass))
#define GTK_IS_FILE_TRANSFER_DIALOG(obj)         (GTK_CHECK_TYPE ((obj), GTK_TYPE_FILE_TRANSFER_DIALOG))
#define GTK_IS_FILE_TRANSFER_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_TRANSFER_DIALOG))

typedef struct _GtkFileTransferDialog            GtkFileTransferDialog;
typedef struct _GtkFileTransferDialogClass       GtkFileTransferDialogClass;

struct _GtkFileTransferDialog {
     GtkDialog parent;
     GaimGtkXferDialog dialog;
};

struct _GtkFileTransferDialogClass {
     GtkDialogClass parent;
     void (*stopped)(GtkFileTransferDialog *, GaimXfer *);
};

enum {
	STOPPED,
	LAST_SIGNAL
};
static guint signals [LAST_SIGNAL] = { 0 };

enum
{
	COLUMN_STATUS = 0,
	COLUMN_PROGRESS,
	COLUMN_FILENAME,
	COLUMN_SIZE,
	COLUMN_REMAINING,
	COLUMN_DATA,
	NUM_COLUMNS
};


typedef struct
{
	GtkTreeIter iter;
	time_t start_time;
	time_t end_time;
	gboolean in_list;

	char *name;

} GaimGtkXferUiData;
static void
get_xfer_info_strings(GaimXfer *xfer, char **kbsec, char **time_elapsed,
					  char **time_remaining)
{
	GaimGtkXferUiData *data;
	double kb_sent, kb_rem;
	double kbps = 0.0;
	time_t elapsed, now;

	data = xfer->ui_data;

	if (data->end_time == -1 &&
		(gaim_xfer_is_canceled(xfer) || gaim_xfer_is_completed(xfer)))
		data->end_time = time(NULL);

	if (data->end_time != -1)
		now = data->end_time;
	else
		now = time(NULL);

	kb_sent = gaim_xfer_get_bytes_sent(xfer) / 1024.0;
	kb_rem  = gaim_xfer_get_bytes_remaining(xfer) / 1024.0;
	elapsed = (now - data->start_time);
	kbps    = (elapsed > 0 ? (kb_sent / elapsed) : 0);

	if (kbsec != NULL) {
		if (gaim_xfer_is_completed(xfer))
			*kbsec = g_strdup("");
		else
			*kbsec = g_strdup_printf(_("%.2f KB/s"), kbps);
	}

	if (time_elapsed != NULL) {
		int h, m, s;
		int secs_elapsed;

		secs_elapsed = now - data->start_time;

		h = secs_elapsed / 3600;
		m = (secs_elapsed % 3600) / 60;
		s = secs_elapsed % 60;

		*time_elapsed = g_strdup_printf("%d:%02d:%02d", h, m, s);
	}

	if (time_remaining != NULL) {
		if (gaim_xfer_get_size(xfer) == 0) {
			*time_remaining = g_strdup(_("Unknown"));
		}
		else if (gaim_xfer_is_completed(xfer)) {
			*time_remaining = g_strdup(_("Finished"));
		}
		else if (gaim_xfer_is_canceled(xfer)) {
			*time_remaining = g_strdup(_("Canceled"));
		}
		else if (kb_sent <= 0) {
			*time_remaining = g_strdup(_("Waiting for transfer to begin"));
		}
		else {
			int h, m, s;
			int secs_remaining;

			secs_remaining = (int)(kb_rem / kbps);

			h = secs_remaining / 3600;
			m = (secs_remaining % 3600) / 60;
			s = secs_remaining % 60;

			*time_remaining = g_strdup_printf("%d:%02d:%02d", h, m, s);
		}
	}
}

static void
update_detailed_info(GaimGtkXferDialog *dialog, GaimXfer *xfer)
{
	GaimGtkXferUiData *data;
	char *kbsec, *time_elapsed, *time_remaining;
	char *status, *utf8;

	if (dialog == NULL || xfer == NULL)
		return;

	data = xfer->ui_data;

	get_xfer_info_strings(xfer, &kbsec, &time_elapsed, &time_remaining);

	status = g_strdup_printf("%ld of %ld",
							 (unsigned long)gaim_xfer_get_bytes_sent(xfer),
							 (unsigned long)gaim_xfer_get_size(xfer));

	if (gaim_xfer_get_size(xfer) >= 0 &&
		gaim_xfer_is_completed(xfer)) {

		GdkPixbuf *pixbuf = NULL;

		pixbuf = gtk_widget_render_icon(ft_dialog->window,
										GAIM_STOCK_FILE_DONE,
										GTK_ICON_SIZE_MENU, NULL);

		gtk_list_store_set(GTK_LIST_STORE(ft_dialog->model), &data->iter,
						   COLUMN_STATUS, pixbuf,
						   -1);

		g_object_unref(pixbuf);
	}

	if (gaim_xfer_get_type(xfer) == GAIM_XFER_RECEIVE) {
		gtk_label_set_markup(GTK_LABEL(dialog->local_user_desc_label),
							 _("<b>Receiving As:</b>"));
		gtk_label_set_markup(GTK_LABEL(dialog->remote_user_desc_label),
							 _("<b>Receiving From:</b>"));
	}
	else {
		gtk_label_set_markup(GTK_LABEL(dialog->remote_user_desc_label),
							 _("<b>Sending To:</b>"));
		gtk_label_set_markup(GTK_LABEL(dialog->local_user_desc_label),
							 _("<b>Sending As:</b>"));
	}

	gtk_label_set_text(GTK_LABEL(dialog->local_user_label), 
								 gaim_account_get_username(xfer->account));
	gtk_label_set_text(GTK_LABEL(dialog->remote_user_label), xfer->who);
	gtk_label_set_text(GTK_LABEL(dialog->protocol_label), 
								 gaim_account_get_protocol_name(xfer->account));

	if (gaim_xfer_get_type(xfer) == GAIM_XFER_RECEIVE) {
		gtk_label_set_text(GTK_LABEL(dialog->filename_label),
					   gaim_xfer_get_filename(xfer));
	} else {
		char *tmp;

		tmp = g_path_get_basename(gaim_xfer_get_local_filename(xfer));
		utf8 = g_filename_to_utf8(tmp, -1, NULL, NULL, NULL);
		g_free(tmp);

		gtk_label_set_text(GTK_LABEL(dialog->filename_label), utf8);
		g_free(utf8);
	}

	utf8 = g_filename_to_utf8((gaim_xfer_get_local_filename(xfer)), -1, NULL, NULL, NULL);
	gtk_label_set_text(GTK_LABEL(dialog->localfile_label), utf8);
	g_free(utf8);

	gtk_label_set_text(GTK_LABEL(dialog->status_label), status);

	gtk_label_set_text(GTK_LABEL(dialog->speed_label), kbsec);
	gtk_label_set_text(GTK_LABEL(dialog->time_elapsed_label), time_elapsed);
	gtk_label_set_text(GTK_LABEL(dialog->time_remaining_label),
					   time_remaining);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(dialog->progress),
								  gaim_xfer_get_progress(xfer));

	g_free(kbsec);
	g_free(time_elapsed);
	g_free(time_remaining);
	g_free(status);
}

static void
stop_button_cb(GtkButton *button, GtkFileTransferDialog *dialog)
{
	g_signal_emit(dialog, signals[STOPPED], 0, dialog->dialog.selected_xfer);
}


static void
selection_changed_cb(GtkTreeSelection *selection, GaimGtkXferDialog *dialog)
{
	GtkTreeIter iter;
	GaimXfer *xfer = NULL;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		GValue val = {0, };

		gtk_tree_model_get_value(GTK_TREE_MODEL(dialog->model),
								 &iter, COLUMN_DATA, &val);

		xfer = g_value_get_pointer(&val);

		update_detailed_info(dialog, xfer);

		dialog->selected_xfer = xfer;
	}
	else {
		dialog->selected_xfer = NULL;
	}
}


static GtkWidget *
setup_tree(GaimGtkXferDialog *dialog)
{
	GtkWidget *sw;
	GtkWidget *tree;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

	/* Create the scrolled window. */
	sw = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
										GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
								   GTK_POLICY_AUTOMATIC,
								   GTK_POLICY_ALWAYS);
	gtk_widget_show(sw);

	/* Build the tree model */
	/* Transfer type, Progress Bar, Filename, Size, Remaining */
	model = gtk_list_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_DOUBLE,
							   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
							   G_TYPE_POINTER);
	dialog->model = model;

	/* Create the treeview */
	dialog->tree = tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));


	gtk_widget_show(tree);

	g_signal_connect(G_OBJECT(selection), "changed",
					 G_CALLBACK(selection_changed_cb), dialog);

	g_object_unref(G_OBJECT(model));


	/* Columns */

	/* Transfer Type column */
	renderer = gtk_cell_renderer_transfer_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
				"pixbuf", COLUMN_STATUS, 
				"name", COLUMN_FILENAME,
				"progress", COLUMN_PROGRESS,
				"size", COLUMN_SIZE,
				"remaining", COLUMN_REMAINING, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(tree));

	gtk_container_add(GTK_CONTAINER(sw), tree);
	gtk_widget_show(tree);

	return sw;
}


static GtkWidget *
make_info_table(GaimGtkXferDialog *dialog)
{
	GtkWidget *hbox, *aspect_frame;
	GtkSizeGroup *size_group = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	GtkWidget *table;
	GtkWidget *label;
	int i;

	struct
	{
		GtkWidget **desc_label;
		GtkWidget **val_label;
		const char *desc;

	} labels[] =
	{
		{ &dialog->local_user_desc_label, &dialog->local_user_label, NULL },
		{ &dialog->remote_user_desc_label, &dialog->remote_user_label, NULL },
		{ &label, &dialog->protocol_label,       _("Protocol:") },
		{ &label, &dialog->filename_label,       _("Filename:") },
		{ &label, &dialog->localfile_label,      _("Local File:") },
		{ &label, &dialog->status_label,         _("Status:") },
		{ &label, &dialog->speed_label,          _("Speed:") },
		{ &label, &dialog->time_elapsed_label,   _("Time Elapsed:") },
		{ &label, &dialog->time_remaining_label, _("Time Remaining:") }
	};

	/* Setup the initial table */
	hbox = gtk_hbox_new(6, FALSE);
	aspect_frame = gtk_aspect_frame_new(NULL, 0, 0, 1, FALSE);
	gtk_frame_set_shadow_type(GTK_FRAME(aspect_frame), GTK_SHADOW_NONE);
	gtk_box_pack_start(GTK_BOX(hbox), aspect_frame, TRUE, TRUE, 0);
	dialog->table = table = gtk_table_new(10, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 0);

	gtk_table_set_row_spacings(GTK_TABLE(table), 6);
	gtk_table_set_col_spacings(GTK_TABLE(table), 6);

	/* Setup the labels */
	for (i = 0; i < sizeof(labels) / sizeof(*labels); i++) {
		GtkWidget *label;
		char buf[256];

		g_snprintf(buf, sizeof(buf), "<b>%s</b>",
			   labels[i].desc != NULL ? labels[i].desc : "");

		*labels[i].desc_label = label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label), buf);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i + 1,
						 GTK_FILL, 0, 0, 0);
		gtk_widget_show(label);

		*labels[i].val_label = label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, i, i + 1,
						 GTK_FILL | GTK_EXPAND, 0, 0, 0);
		gtk_widget_show(label);
	}

	/* Setup the progress bar */
	dialog->progress = gtk_pie_chart_new();	
	gtk_size_group_add_widget(size_group, dialog->progress);
	gtk_size_group_add_widget(size_group, table);
	gtk_container_add(GTK_CONTAINER(aspect_frame), dialog->progress);
	gtk_widget_show(dialog->progress);

	gtk_widget_show_all(hbox);
	return hbox;
}

static void gtk_file_transfer_dialog_class_init (GtkFileTransferDialogClass *ft_class)
{
	GObjectClass *gobject_class = (GObjectClass*)ft_class;
	signals[STOPPED] = g_signal_new("stopped",
					G_TYPE_FROM_CLASS(gobject_class),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET(GtkFileTransferDialogClass, stopped),
					NULL,
					0,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE, 1,
					G_TYPE_POINTER);
}

static void gtk_file_transfer_dialog_init (GtkFileTransferDialog *ft_dialog)
{
     GaimGtkXferDialog *dialog;
	GtkWidget *window;
	GtkWidget *vbox2;

	GtkWidget *bbox;
	GtkWidget *sw;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *checkbox;

	dialog = &(ft_dialog->dialog);
	dialog->keep_open =
		gaim_prefs_get_bool("/gaim/gtk/filetransfer/keep_open");
	dialog->auto_clear =
		gaim_prefs_get_bool("/gaim/gtk/filetransfer/clear_finished");

	/* Create the window. */
	dialog->window = window = GTK_WIDGET(ft_dialog);
	gtk_window_set_role(GTK_WINDOW(window), "file transfer");
	gtk_window_set_title(GTK_WINDOW(window), _("File Transfers"));
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 12);
	g_signal_connect(G_OBJECT(ft_dialog), "delete_event", G_CALLBACK(gtk_widget_hide), NULL);
	vbox2 = GTK_DIALOG(ft_dialog)->vbox;
	bbox =  GTK_DIALOG(ft_dialog)->action_area;


	/* Setup the listbox */
	sw = setup_tree(dialog);
	gtk_box_pack_start(GTK_BOX(vbox2), sw, TRUE, TRUE, 0);
	gtk_widget_set_size_request(sw,-1, 140);

	/* "Keep the dialog open" */
	checkbox = gtk_check_button_new_with_mnemonic(
			_("_Keep the dialog open"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
								 dialog->keep_open);

	gtk_box_pack_start(GTK_BOX(vbox2), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	/* "Clear finished transfers" */
	checkbox = gtk_check_button_new_with_mnemonic(
			_("_Clear finished transfers"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
								 dialog->auto_clear);
	gtk_box_pack_start(GTK_BOX(vbox2), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	/* The table of information. */
	table = make_info_table(dialog);
	gtk_box_pack_start(GTK_BOX(vbox2), table, TRUE, TRUE, 0);

	/* Open button */
	button = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_widget_set_sensitive(button, FALSE);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	dialog->open_button = button;

	/* Pause button */
	button = gtk_button_new_with_mnemonic(_("_Pause"));
	gtk_widget_set_sensitive(button, FALSE);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	dialog->pause_button = button;

	/* Resume button */
	button = gtk_button_new_with_mnemonic(_("_Resume"));
	gtk_widget_set_sensitive(button, FALSE);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	dialog->resume_button = button;

	/* Remove button */
	button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_hide(button);
	dialog->remove_button = button;

	/* Stop button */
	button = gtk_button_new_from_stock(GTK_STOCK_STOP);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	dialog->stop_button = button;
	g_signal_connect(G_OBJECT(button), "clicked",
					 G_CALLBACK(stop_button_cb), ft_dialog);

	/* Close button */
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	dialog->close_button = button;
}

GType gtk_file_transfer_dialog_get_type()
{
	static GType file_transfer_dialog_type = 0;

	if (!file_transfer_dialog_type) {
		static const GTypeInfo file_transfer_dialog_info = {
			sizeof(GtkFileTransferDialogClass),
			NULL,
			NULL,
			(GClassInitFunc) gtk_file_transfer_dialog_class_init,
			NULL,
			NULL,
			sizeof (GtkFileTransferDialog),
			0,
			(GInstanceInitFunc) gtk_file_transfer_dialog_init
		};

		file_transfer_dialog_type = g_type_register_static(GTK_TYPE_DIALOG,
				"GtkFileTransferDialog", &file_transfer_dialog_info, 0);
	}

	return file_transfer_dialog_type;
}

GtkWidget *gtk_file_transfer_dialog_new(GaimGtkXferDialog *dialog)
{
     GtkWidget *widget = GTK_WIDGET(g_object_new(gtk_file_transfer_dialog_get_type(), NULL));
//     memcpy(dialog, &(GTK_FILE_TRANSFER_DIALOG(widget)->dialog), sizeof(GaimGtkXferDialog));
     return widget;
}


/*******************************************************************
 * Plugin Code *****************************************************
 *******************************************************************/

static void
stopped_cb (GtkWidget *widget, GaimXfer *xfer)
{
	gaim_notify_info(NULL, NULL, "You stopped this transfer", NULL);
}

static gboolean
plugin_load(GaimPlugin *plugin)
{
     GaimGtkXferDialog *old_dialog = gaim_get_gtkxfer_dialog();
     GtkWidget *dialog = gtk_file_transfer_dialog_new(NULL);
     g_signal_connect(G_OBJECT(dialog), "stopped", G_CALLBACK(stopped_cb), NULL);
     if (old_dialog)
	     gaim_gtkxfer_dialog_destroy(old_dialog);
     gaim_set_gtkxfer_dialog(&(GTK_FILE_TRANSFER_DIALOG(dialog)->dialog));
     plugin_handle = plugin;
     return TRUE;
}

static GaimPluginInfo info =
{
     GAIM_PLUGIN_MAGIC,
     GAIM_MAJOR_VERSION,
     GAIM_MINOR_VERSION,     
     GAIM_PLUGIN_STANDARD,
     NULL,
     0,
     NULL,
     GAIM_PRIORITY_DEFAULT,
     "ftmakeover",
     "File Transfer UI Makeover",
     "1.0",
     "Adds eye-candy to the file transfer window",
     "This plugin replaces the file transfer window "
     "with a composite GTK+ widget, which uses a custom "
     "cell renderer and custom piechart widget",
     "Sean Egan <seanegan@gmail.com>",
     "http://apress.com",
     plugin_load,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL
};

 static void
 init_plugin(GaimPlugin *plugin)
 {
 }

GAIM_INIT_PLUGIN(urlcatcher, init_plugin, info)
