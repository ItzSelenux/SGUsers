#include "sgusers.h"

int main(int argc, char *argv[])
{
int nocsd = 0;

for(int i = 1; i < argc; i++)
{
	if(strcmp(argv[i], "--nocsd") == 0)
	{
	nocsd = 1;
	printf("CSD Disabled, using fallback display \n");
	}
}

	gtk_init(&argc, &argv);
	if (argc > 0) 
	{
		pm = argv[0];
	}
	else 
	{
		pm = "sgrandr";
	}

	GPtrArray *group_array = g_ptr_array_new_with_free_func(free_group_info);
	if (!read_group_info(group_array))
	{
		g_ptr_array_free(group_array, TRUE);
		return 1;
	}

	// Create the main window
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "SGUsers");
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "cs-user", 48, 0);
	if (info != NULL) 
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(window), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}
	//accelerator header
	GtkAccelGroup *accel_group = gtk_accel_group_new();
		gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	// Create the header bar
	GtkWidget *headerbar = gtk_header_bar_new();
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), TRUE);

	// Create the button with an icon
	GtkWidget *button = gtk_menu_button_new();
	GtkWidget *wicon = gtk_image_new_from_icon_name("cs-user", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(button), wicon);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), button);

	// Create the title
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<b>Users Management - SGUsers</b>");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), label);

	// Create the submenu
	GtkWidget *submenu = gtk_menu_new();

	// Create the submenu items

	GtkWidget *submenu_item1 = gtk_menu_item_new_with_label("See system's groups list");
	GtkWidget *submenu_item2 = gtk_menu_item_new_with_label("Reload Program");
	GtkWidget *submenu_item4 = gtk_menu_item_new_with_label("Manage root account");
	GtkWidget *submenu_item3 = gtk_menu_item_new_with_label("About");


	// Add the submenu items to the submenu
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item1);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item2);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item4);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item3);

	// Show all the submenu items
	gtk_widget_show_all(submenu);

	// Connect the button to the submenu
	gtk_menu_button_set_popup(GTK_MENU_BUTTON(button), submenu);


	if (nocsd == 0 )
	{
	// Add the header bar to the main window
	gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
	}

GtkWidget *grid;
	grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
	gtk_container_add(GTK_CONTAINER(window), grid);


	GtkWidget *button_add = gtk_button_new_with_label("Add Users");
	g_signal_connect(button_add, "clicked", G_CALLBACK(add_user), NULL);
	gtk_grid_attach(GTK_GRID(grid), button_add, 0, 0, 2, 1);

	FILE *fp;
	char buffer[256];
	fp = popen("cut -d: -f1 /etc/passwd", "r");
	if (fp != NULL) 
	{
		int row = 1; // funny bug with positions
		while (fgets(buffer, sizeof(buffer), fp)) 
		{
			buffer[strcspn(buffer, "\n")] = 0;
			struct passwd *pw = getpwnam(buffer);
			if (pw && pw->pw_uid >= 1000 && pw->pw_uid <= 65533) 
			{
				char label_text[256];
				snprintf(label_text, sizeof(label_text), "%s (%d)", buffer, pw->pw_uid);

				// handle different usernames
				char *user_name = g_strdup(buffer);

				GtkWidget *button_rename = gtk_button_new_with_label("Edit");
				g_signal_connect(button_rename, "clicked", G_CALLBACK(edit_user), user_name);

				GtkWidget *label = gtk_label_new(label_text);

				gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
				gtk_grid_attach(GTK_GRID(grid), button_rename, 1, row, 1, 1);
				row++;
			}
		}
	pclose(fp);
}

	g_signal_connect(submenu_item1, "activate", G_CALLBACK(on_submenu_item1_selected),  group_array);
	g_signal_connect(submenu_item2, "activate", G_CALLBACK(restart_program), pm);
	g_signal_connect(submenu_item3, "activate", G_CALLBACK(on_submenu_item3_selected), NULL);
	g_signal_connect(submenu_item4, "activate", G_CALLBACK(on_submenu_item4_selected), NULL);


	// Keybinds
	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press), submenu);
	gtk_widget_add_accelerator(submenu_item1, "activate", accel_group, GDK_KEY_G, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(submenu_item2, "activate", accel_group, GDK_KEY_R, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(submenu_item4, "activate", accel_group, GDK_KEY_R, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

	gtk_widget_add_accelerator(button_add, "activate", accel_group, GDK_KEY_A, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(window);
	
	if (!is_user_root())
	{
		show_error_dialog(window);
		gtk_widget_destroy(window);
		return 0;
	}
	
	gtk_main();
	g_ptr_array_free(group_array, TRUE);
	return 0;
}
