#include <gtk/gtk.h>
#include <stdlib.h>

extern	GtkWidget *main_window;
extern	GtkWidget *vbox_main, *hbox_main, *hbox_bottom;
extern	GtkWidget *exit_button, *send_button;
extern	GtkWidget *main_text, *roster_text, *text_buffer;	
extern	GtkWidget *entry; 
extern	GtkWidget *scroll_mt, *scroll_rt;


gint delete_event( GtkWidget *widget, GdkEvent *event, gpointer data );
void destroy( GtkWidget *widget, gpointer data );
void gtk_send_button_clicked(GtkButton *button);
int g_chat_form (int argc, char **argv);

extern	GtkWidget *settings_window;
extern	GtkWidget *s_vbox_main;
extern	GtkWidget *s_hbox_serv, *e_serv, *l_serv;
extern	GtkWidget *s_hbox_user, *e_user, *l_user;
extern	GtkWidget *s_hbox_pass, *e_pass, *lpass;
extern	GtkWidget *s_hbox_muc_nick, *e_muc_nick, *l_muc_nick;
extern	GtkWidget *s_hbox_jiv_name, *e_jiv_name, *l_jiv_name;
extern	GtkWidget *s_hbox_jiv_os, *e_jiv_os,*l_jiv_os ;
extern	GtkWidget *s_hbox_jiv_version, *e_jiv_version, *l_jiv_version ;
extern	GtkWidget *s_hbox_proxy_serv, *e_proxy_serv, *l_proxy_serv ;
extern	GtkWidget *s_hbox_proxy_port, *e_proxy_port, *l_proxy_port ;
extern	GtkWidget *s_hbox_proxy_user, *e_proxy_user, *l_proxy_user ;
extern	GtkWidget *s_hbox_proxy_pass, *e_proxy_pass, *l_proxy_pass ;
extern	GtkWidget *s_hbox_jiv_version, *e_jiv_version, *l_jiv_version ;			
extern	GtkWidget *s_hbox_bottom, *bt_s_ok, *bt_exit;

void gtk_gtk_bt_s_ok_clicked_clicked(GtkButton *button);
