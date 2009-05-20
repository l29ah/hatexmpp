#include <gtk/gtk.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <gdk/gdk.h>


/*Виджеты используемые в окне чата*/
	GtkWidget *chat_window;
	GtkWidget *vbox_main, *hbox_main, *hbox_bottom;
	GtkWidget *send_button;
	GtkWidget *main_text, *roster_text, *text_buffer;	
	GtkWidget *entry; 
	GtkWidget *scroll_mt, *scroll_rt;
	
	
	
//Если нажать на крестик - окно закроется
gint delete_event( GtkWidget *widget, GdkEvent *event, gpointer data )
{
	gtk_main_quit ();
}

//Обработчик события нажатия на кнопку "Отправить"

void gtk_send_button_clicked(GtkButton *button)
{
	char *str;
	str = gtk_entry_get_text(GTK_ENTRY(entry));	
	send_message("fs/roster/anime@conference.jabber.ru/__chat", str);
	gtk_entry_set_text(entry, "");
} 


//Функция рисует окно чата
int g_chat_form (int argc, char **argv)
{
	gtk_init (&argc, &argv);
	
	chat_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		vbox_main = gtk_vbox_new (FALSE, 1);
			hbox_main = gtk_hbox_new (FALSE, 1);
				scroll_mt = gtk_scrolled_window_new(NULL, NULL);
					main_text = gtk_text_view_new(); 
				scroll_rt = gtk_scrolled_window_new(NULL, NULL);
					roster_text = gtk_tree_view_new();	
			hbox_bottom = gtk_hbox_new (FALSE, 1);
				entry = gtk_entry_new();	
				send_button = gtk_button_new_with_label("Отправить");
	text_buffer = gtk_text_buffer_new(NULL);

/*Сигналы*/
	//этот сгнал грохнет окошко, наверное его стоит отключить
	gtk_signal_connect (GTK_OBJECT (chat_window), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);
	//Сигнал нажатия на кнопку отправить
	gtk_signal_connect (GTK_OBJECT (send_button), "clicked", 
		GTK_SIGNAL_FUNC (gtk_send_button_clicked), NULL);


/*Вот оно - построение виджетов*/
	//свойства окна чата
	gtk_window_set_default_size (GTK_WINDOW (chat_window), 800, 600);
	gtk_window_set_title (GTK_WINDOW (chat_window), "gHateXMPP");
	
	// Упаковка виджетов один в другой
	gtk_box_pack_start (GTK_BOX (vbox_main), hbox_main, TRUE, TRUE,1);	
		gtk_box_pack_start (GTK_BOX (hbox_main), scroll_mt, TRUE, TRUE,1);
			gtk_container_add (GTK_CONTAINER (scroll_mt), main_text );
			
		gtk_box_pack_start (GTK_BOX (hbox_main), scroll_rt, 0, 0,1);
			gtk_widget_set_size_request (GTK_WIDGET(roster_text), 200, -1  );
			gtk_container_add (GTK_CONTAINER (scroll_rt), roster_text );
			
	gtk_box_pack_start (GTK_BOX (vbox_main), hbox_bottom, FALSE, FALSE,1);
		gtk_box_pack_start (GTK_BOX (hbox_bottom), entry, TRUE, TRUE,1);
		gtk_box_pack_start (GTK_BOX (hbox_bottom), send_button	, FALSE, FALSE,1);
			
						
	gtk_container_add (GTK_CONTAINER (chat_window), vbox_main);	//так надо
	gtk_widget_show_all (chat_window);
	gtk_main();

	return 0;
}




/*
 * Снова виджеты, но уже для окна настроек
 * поудалять бы половину, но я не знаю,
 * что должно остаться в настройках
 */
	GtkWidget *settings_window;
		GtkWidget *s_vbox_main;
			GtkWidget *s_hbox_serv, *e_serv, *l_serv;
			GtkWidget *s_hbox_user, *e_user, *l_user;
			GtkWidget *s_hbox_pass, *e_pass, *l_pass;
			GtkWidget *s_hbox_muc_nick, *e_muc_nick, *l_muc_nick;
			GtkWidget *s_hbox_jiv_name, *e_jiv_name, *l_jiv_name;
			GtkWidget *s_hbox_jiv_os, *e_jiv_os,*l_jiv_os ;
			GtkWidget *s_hbox_jiv_version, *e_jiv_version, *l_jiv_version ;
		/*	GtkWidget *s_hbox_proxy_serv, *e_proxy_serv, *l_proxy_serv ;
			GtkWidget *s_hbox_proxy_port, *e_proxy_port, *l_proxy_port ;
			GtkWidget *s_hbox_proxy_user, *e_proxy_user, *l_proxy_user ;
			GtkWidget *s_hbox_proxy_pass, *e_proxy_pass, *l_proxy_pass ;
		*/
			GtkWidget *s_hbox_jiv_version, *e_jiv_version, *l_jiv_version ;			
			GtkWidget *s_hbox_bottom, *bt_s_ok, *bt_exit;	

//Обработчик события нажатия на кнопку "далее" на форме настроек
void gtk_bt_s_ok_clicked(GtkButton *button, int argc, char **argv)
{
	int folder;
	char *serv, *user, *pass, *muc_nick;
	char *jiv_name, *jiv_os, *jiv_version;
//	char *proxy_serv, *proxy_port, *proxy_user, *proxy_pass;
	//Считываем настройки
	serv = gtk_entry_get_text(e_serv);	
	user = gtk_entry_get_text(e_user);
	pass = gtk_entry_get_text(e_pass);
	muc_nick = gtk_entry_get_text(e_muc_nick);
	jiv_name = gtk_entry_get_text(e_jiv_name);
	jiv_os = gtk_entry_get_text(e_jiv_os);
	jiv_version = gtk_entry_get_text(e_jiv_version);
/*	proxy_serv = gtk_entry_get_text(e_proxy_serv);
	proxy_port = gtk_entry_get_text(e_proxy_port);
	proxy_user = gtk_entry_get_text(e_proxy_user);
	proxy_pass = gtk_entry_get_text(e_proxy_pass);
*/
	//Записываем настройки

	write_settings_to_file("fs/config/server", serv);
	write_settings_to_file("fs/config/username", user);
	write_settings_to_file("fs/config/password", pass);
	write_settings_to_file("fs/config/muc_default_nick", muc_nick);
	write_settings_to_file("fs/config/jiv_name", jiv_name);
	write_settings_to_file("fs/config/jiv_os", jiv_os);
	write_settings_to_file("fs/config/jiv_version", jiv_version);
/*	write_settings_to_file("fs/config/proxy_serv", proxy_serv);
	write_settings_to_file("fs/config/proxy_serv", proxy_serv);
	write_settings_to_file("fs/config/proxy_serv", proxy_serv);
	write_settings_to_file("fs/config/proxy_serv", proxy_serv);
*/
	//прячем само окно
	gtk_widget_hide(GTK_WINDOW (settings_window));
	roster_form (&argc, &argv);

} 
	
	
	
//Это если приспичило нажать на кнопку выхода	
void settings_window_destroy( GtkWidget *widget, gpointer data )
{
	g_print("Выход из окна настроек\n");
	exit(0);
}	
	
	
	
	
//Вот оно, окно настроек	
int g_settings_form (int argc, char **argv)
{
	gtk_init (&argc, &argv);
	
	settings_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		s_vbox_main = gtk_vbox_new (TRUE, 1);
			s_hbox_serv = gtk_hbox_new (FALSE, 1);
				e_serv = gtk_entry_new();
				l_serv = gtk_label_new("server");
			s_hbox_user = gtk_hbox_new (FALSE, 1);
				e_user = gtk_entry_new();
				l_user = gtk_label_new("user");				
			s_hbox_pass = gtk_hbox_new (FALSE, 1);
				e_pass = gtk_entry_new();
				l_pass = gtk_label_new("password");
			s_hbox_muc_nick = gtk_hbox_new (FALSE, 1);
				e_muc_nick = gtk_entry_new();
				l_muc_nick = gtk_label_new("muc_nick");
			s_hbox_jiv_name = gtk_hbox_new (FALSE, 1);
				e_jiv_name = gtk_entry_new();
				l_jiv_name = gtk_label_new("jiv_name");
			s_hbox_jiv_os = gtk_hbox_new (FALSE, 1);
				e_jiv_os = gtk_entry_new();
				l_jiv_os = gtk_label_new("jiv_os");
			s_hbox_jiv_version = gtk_hbox_new (FALSE, 1);
				e_jiv_version = gtk_entry_new();
				l_jiv_version = gtk_label_new("jiv_vers");
/*			s_hbox_proxy_serv = gtk_hbox_new (FALSE, 1);
				e_proxy_serv = gtk_entry_new();
				l_proxy_serv = gtk_label_new("proxy serv");
			s_hbox_proxy_port = gtk_hbox_new (FALSE, 1);
				e_proxy_port = gtk_entry_new();
				l_proxy_port = gtk_label_new("proxy port");
			s_hbox_proxy_user = gtk_hbox_new (FALSE, 1);
				e_proxy_user = gtk_entry_new();
				l_proxy_user = gtk_label_new("proxy user");
			s_hbox_proxy_pass = gtk_hbox_new (FALSE, 1);
				e_proxy_pass = gtk_entry_new();
				l_proxy_pass = gtk_label_new("proxy pass");
*/
			s_hbox_bottom = gtk_hbox_new (TRUE, 1);			
				bt_s_ok = gtk_button_new_with_label("Далее");
				bt_exit = gtk_button_new_with_label("Выход");
		
			
/*Сигналы*/		
		//аналогично
	gtk_signal_connect (GTK_OBJECT (settings_window), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	gtk_signal_connect (GTK_OBJECT (settings_window), "destroy",
		GTK_SIGNAL_FUNC (settings_window_destroy), NULL);

	gtk_signal_connect_object (GTK_OBJECT (bt_exit), "clicked",
		GTK_SIGNAL_FUNC (gtk_widget_destroy),
		GTK_OBJECT (settings_window));
		
	gtk_signal_connect (GTK_OBJECT (bt_s_ok), "clicked", 
		GTK_SIGNAL_FUNC (gtk_bt_s_ok_clicked), NULL);
		
/*Немного магии построения*/		
	gtk_window_set_default_size (GTK_WINDOW (settings_window), 300, 400);
	gtk_window_set_title (GTK_WINDOW (settings_window), "Settings");
	
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_serv, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_serv), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_serv), e_serv, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_serv), l_serv, TRUE, TRUE,1);
		
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_user, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_user), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_user), e_user, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_user), l_user, TRUE, TRUE,1);
			
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_pass, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_pass), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_pass), e_pass, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_pass), l_pass, TRUE, TRUE,1);
			
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_muc_nick, TRUE, TRUE,1);
			gtk_widget_set_size_request (GTK_WIDGET(l_muc_nick), 50, -1  );	
			gtk_box_pack_start (GTK_BOX (s_hbox_muc_nick), e_muc_nick, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_muc_nick), l_muc_nick, TRUE, TRUE,1);	
				
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_jiv_name, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_jiv_name), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_name), e_jiv_name, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_name), l_jiv_name, TRUE, TRUE,1);	
				
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_jiv_os, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_jiv_os), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_os), e_jiv_os, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_os), l_jiv_os, TRUE, TRUE,1);	
				
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_jiv_version, TRUE, TRUE,1);
			gtk_widget_set_size_request (GTK_WIDGET(l_jiv_version), 50, -1  );	
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_version), e_jiv_version, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_jiv_version), l_jiv_version, TRUE, TRUE,1);		
			
/*		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_proxy_serv, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_proxy_serv), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_serv), e_proxy_serv, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_serv), l_proxy_serv, TRUE, TRUE,1);		
			
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_proxy_port, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_proxy_port), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_port), e_proxy_port, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_port), l_proxy_port, TRUE, TRUE,1);		
			
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_proxy_user, TRUE, TRUE,1);	
			gtk_widget_set_size_request (GTK_WIDGET(l_proxy_user), 50, -1  );
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_user), e_proxy_user, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_user), l_proxy_user, TRUE, TRUE,1);		
			
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_proxy_pass, TRUE, TRUE,1);
			gtk_widget_set_size_request (GTK_WIDGET(l_proxy_pass), 50, -1  );	
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_pass), e_proxy_pass, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_proxy_pass), l_proxy_pass, TRUE, TRUE,1);		
*/		
		gtk_box_pack_start (GTK_BOX (s_vbox_main), s_hbox_bottom, TRUE, TRUE,1);	
			gtk_box_pack_start (GTK_BOX (s_hbox_bottom), bt_s_ok, TRUE, TRUE,1);
			gtk_box_pack_start (GTK_BOX (s_hbox_bottom), bt_exit, TRUE, TRUE,1);		
		
	gtk_container_add (GTK_CONTAINER (settings_window), s_vbox_main);
	gtk_widget_show_all (settings_window);
		
	gtk_main();

	return 0;
}






/* Виджеты для третьего окна:
 * по аналогии с gajim - там оно есть такое.
 * Возможно в нём будет удобно отображать
 */ 
	GtkWidget *roster_window;
	GtkWidget *r_vbox_main, *r_hboh_bottom, *r_menu_bar;
	GtkWidget *r_entry, *r_bt_connect;
	GtkWidget *r_text, *r_text_buffer;	
	GtkWidget *r_scroll;
	GtkWidget *r_menu_1, *r_menu_2, *r_menu_3, *r_menu_4;
	GtkWidget *sub_1;
	
//Какая же программа без менюбара с элементами?
	
void gtk_r_menu_3_clicked(GtkMenuItem *menuitem)
{
	int folder;
	folder = mkdir("fs/roster/anime@conference.jabber.ru",S_IRWXU );
} 

void gtk_r_menu_4_clicked(GtkMenuItem *menuitem)
{
	g_print("Выход\n");
	exit(0);		
} 


void show_roster()
{       
	struct dirent **namelist;
	int n = 2;
	int m;
    m = scandir("fs/roster", &namelist, 0, alphasort);
	if (m < 0)
		perror("scandir");
    else 
	{
		while (n < m) 
			{
				printf("%s\n", namelist[n]->d_name);
				n++;
				//free(namelist[n]);
			}
        //free(*namelist);
	}
	g_chat_form (NULL, NULL);
}

void gtk_r_bt_connect_clicked()
{
	int folder;
	char *f_name, c_name[100] = "fs/roster/";
	f_name = gtk_entry_get_text(r_entry);
	folder = mkdir(strcat(c_name, f_name), S_IRWXU);
	gtk_entry_set_text(r_entry, "");
}

int roster_form(int argc, char **argv)
{
	int folder;
	gtk_init (&argc, &argv);
	
	roster_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		r_menu_bar = gtk_menu_bar_new();
		r_vbox_main = gtk_vbox_new (FALSE, 1);
			r_scroll = gtk_scrolled_window_new(NULL, NULL);
				r_text = gtk_tree_view_new(); 
			r_hboh_bottom = gtk_hbox_new (FALSE, 1);
				r_entry = gtk_entry_new();
				r_bt_connect = gtk_button_new_with_label("Соединить");
	r_menu_1 = gtk_menu_item_new_with_label("Бдыщь 1");
		sub_1 = gtk_menu_new(); 
	r_menu_2 = gtk_menu_item_new_with_label("Бдыщь 2");
	r_menu_3 = gtk_menu_item_new_with_label("Бдыщь 3");
	r_menu_4 = gtk_menu_item_new_with_label("Выход");
	
	r_text_buffer = gtk_text_buffer_new(NULL);
	
	/*Сигналы*/
	gtk_signal_connect (GTK_OBJECT (roster_window), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	gtk_signal_connect (GTK_OBJECT (r_menu_3), "activate", 
		GTK_SIGNAL_FUNC (gtk_r_menu_3_clicked), NULL);

	gtk_signal_connect (GTK_OBJECT (r_menu_4), "activate", 
		GTK_SIGNAL_FUNC (gtk_r_menu_4_clicked), NULL);

	gtk_signal_connect (GTK_OBJECT (r_bt_connect), "clicked", 
		GTK_SIGNAL_FUNC (gtk_r_bt_connect_clicked), NULL);		
		
/*Снова магия*/
	gtk_window_set_default_size (GTK_WINDOW (roster_window), 300, 400);
	gtk_window_set_title (GTK_WINDOW (roster_window), "gHateXMPP");
	
	gtk_box_pack_start (GTK_BOX (r_vbox_main), r_menu_bar, FALSE, TRUE,1);
		gtk_menu_shell_insert (r_menu_bar,r_menu_1,1);
			gtk_menu_item_set_submenu (r_menu_1, sub_1);
			gtk_menu_shell_insert(sub_1, r_menu_3, 1);
			gtk_menu_shell_insert(sub_1, r_menu_4, 2);
		gtk_menu_shell_insert (r_menu_bar,r_menu_2,2);

	gtk_box_pack_start (GTK_BOX (r_vbox_main), r_scroll, TRUE, TRUE,1);	
	gtk_box_pack_start (GTK_BOX (r_vbox_main), r_hboh_bottom, FALSE, FALSE,1);
		gtk_box_pack_start (GTK_BOX (r_hboh_bottom), r_entry, TRUE, TRUE,1);
		gtk_box_pack_start (GTK_BOX (r_hboh_bottom), r_bt_connect, FALSE, FALSE,1);
		gtk_container_add (GTK_CONTAINER (r_scroll), r_text);
		gtk_widget_set_size_request (GTK_WIDGET(r_text), -1, 280  );
	gtk_widget_set_size_request (GTK_WIDGET(r_menu_bar), -1, 20  );

	
	gtk_container_add (GTK_CONTAINER (roster_window), r_vbox_main);	
	gtk_widget_show_all (roster_window);
	
	folder = mkdir("fs/roster",S_IRWXU );
	//вызываем окно чата
	show_roster();
	gtk_main();
	return 0;
}

void write_settings_to_file(char *file_name, char *w_phrase)
{
	FILE *w_file;
	if ( (w_file = fopen(file_name, "w")) == NULL )
		{	
			printf("Не могу открыть файл\n");
		}
	fprintf(w_file, w_phrase);
	fclose(w_file);	
}

void send_message(char *file_name, char *w_phrase)
{
	FILE *w_file;
	if ( (w_file = fopen(file_name, "w")) == NULL )
		{	
			printf("Не могу открыть файл\n");
		}
	fprintf(w_file, w_phrase);
	fclose(w_file);	
}
