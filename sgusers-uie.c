#include "sgusers.h"

char checkusername[ML];

int main(int argc, char *argv[])
{
get_username(&*argvusername);
int nocsd = 0;

for(int i = 1; i < argc; i++)
{
	if(strcmp(argv[i], "--nocsd") == 0)
	{
		nocsd = 1;
		printf("CSD Disabled, using fallback display \n");
	}
	 if (argv[i][0] != '-')
	{
			*argvusername = argv[i];
	}
}
	// Check if not username argv
	if (*argvusername == NULL)
	{
		*argvusername = (char*)malloc(strlen("1000") + 1);
		strcpy(*argvusername, "1000");
	}

	printf("username: %s\n", *argvusername);

snprintf(checkusername, sizeof(checkusername), "id %s", *argvusername);
	int idintegrity = system(checkusername);

	if (idintegrity == -1)
	{
		printf("Error validing user.\n");
		return 1;
	}

	if (WIFEXITED(idintegrity))
	{
		int exit_status = WEXITSTATUS(idintegrity);
		if (exit_status == 0)
		{
			printf("Valid user.\n");
		}
		else
		{
			printf("Error, not valid user in system.\n");
			return 1;
		}
	}
	else
	{
		printf("Failed to check user.\n");
		return 1;
	}

	UserData data;
	memset(&data, 0, sizeof(data));
	extractUserData(&data, *argvusername);

	// Print the extracted data
	printf("fname: %s\n", data.fname);
	printf("office: %s\n", data.office);
	printf("phone: %s\n", data.phone);
	printf("officephone: %s\n", data.officephone);
	printf("email: %s\n", data.email);



	gtk_init(&argc, &argv);
	if (argc > 0)
	{
		pm = argv[0];
	}
	else
	{
		pm = "sgusers";
	}

	GPtrArray *group_array = g_ptr_array_new_with_free_func(free_group_info);
	if (!read_group_info(group_array))
	{
		g_ptr_array_free(group_array, TRUE);
		return 1;
	}

	// Create the main window
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "SGUsers-uie");
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
	gtk_label_set_markup(GTK_LABEL(label), "<b>Users Info Editor - SGUsers</b>");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), label);

	// Create the submenu
	GtkWidget *submenu = gtk_menu_new();

	// Create the submenu items

	GtkWidget *submenu_item1 = gtk_menu_item_new_with_label("Change Profile Picture");
	GtkWidget *submenu_item2 = gtk_menu_item_new_with_label("Clear User");
	GtkWidget *submenu_item3 = gtk_menu_item_new_with_label("About");


	// Add the submenu items to the submenu
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item1);
	//gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item2);
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

	GtkWidget *propic = gtk_button_new();

		char default_image_path[256];
		snprintf(default_image_path, sizeof(default_image_path), "/home/%s/.face", *argvusername);
		GdkPixbuf *default_pixbuf = gdk_pixbuf_new_from_file(default_image_path, NULL);
		GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(default_pixbuf, 64, 64, GDK_INTERP_BILINEAR);
		g_object_unref(default_pixbuf);
		GtkWidget *image = gtk_image_new_from_pixbuf(resized_pixbuf);
		gtk_button_set_image(GTK_BUTTON(button), image);
		g_object_unref(resized_pixbuf);

	GtkWidget *fname_label = gtk_label_new("Full Name:");
		GtkWidget *fname_entry = gtk_entry_new();
		gtk_entry_set_placeholder_text(GTK_ENTRY(fname_entry), "Laura Dominguez");

	GtkWidget *phone_label = gtk_label_new("Phone Number:");
		GtkWidget *phone_entry = gtk_entry_new();
		gtk_entry_set_placeholder_text(GTK_ENTRY(phone_entry), "123456789");

	GtkWidget *email_label = gtk_label_new("Email Address:");
		GtkWidget *email_entry  = gtk_entry_new();
		gtk_entry_set_placeholder_text(GTK_ENTRY(email_entry), "user@example.com");

	GtkWidget *office_label = gtk_label_new("Office:");
		GtkWidget *office_entry  = gtk_entry_new();
		gtk_entry_set_placeholder_text(GTK_ENTRY(office_entry), "Example Corporation");

	GtkWidget *ofn_label = gtk_label_new("Office Number:");
		GtkWidget *ofn_entry  = gtk_entry_new();
		gtk_entry_set_placeholder_text(GTK_ENTRY(ofn_entry), "123456789");

	GtkWidget *ApplyBTN = gtk_button_new_with_label("Save");

		gtk_entry_set_text(GTK_ENTRY(fname_entry), data.fname);
		gtk_entry_set_text(GTK_ENTRY(phone_entry), data.phone);
		gtk_entry_set_text(GTK_ENTRY(email_entry), data.email);
		gtk_entry_set_text(GTK_ENTRY(office_entry), data.office);
		gtk_entry_set_text(GTK_ENTRY(ofn_entry), data.officephone);

	gtk_grid_attach(GTK_GRID(grid), fname_label, 0, 1, 2, 1);
	gtk_grid_attach(GTK_GRID(grid), fname_entry, 0, 2, 2, 1);

	gtk_grid_attach(GTK_GRID(grid), phone_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), phone_entry, 0, 4, 1, 1);

	gtk_grid_attach(GTK_GRID(grid), email_label, 1, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), email_entry, 1, 4, 1, 1);

	gtk_grid_attach(GTK_GRID(grid), office_label, 0, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), office_entry, 0, 6, 1, 1);

	gtk_grid_attach(GTK_GRID(grid), ofn_label, 1, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ofn_entry, 1, 6, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), ApplyBTN, 0, 7, 2, 2);

	if (nocsd)
	{
		gtk_grid_attach(GTK_GRID(grid), propic, 0, 0, 2, 1);
	}

	g_object_set_data(G_OBJECT(fname_entry), "phone_entry", phone_entry);
	g_object_set_data(G_OBJECT(fname_entry), "email_entry", email_entry);
	g_object_set_data(G_OBJECT(fname_entry), "office_entry", office_entry);
	g_object_set_data(G_OBJECT(fname_entry), "ofn_entry", ofn_entry);

	// Connect the button's "clicked" signal to the save data function
	g_signal_connect(G_OBJECT(ApplyBTN), "clicked", G_CALLBACK(uie_savedata), (gpointer)fname_entry);

	g_signal_connect(propic, "clicked", G_CALLBACK(on_pfimage_clicked), propic);
	g_signal_connect(submenu_item1, "activate", G_CALLBACK(on_pfimage_clicked), button);
	//g_signal_connect(submenu_item2, "activate", G_CALLBACK(restart_program), pm);
	g_signal_connect(submenu_item3, "activate", G_CALLBACK(on_submenu_item3_selected), NULL);

	// Keybinds
	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press), submenu);
	gtk_widget_add_accelerator(submenu_item1, "activate", accel_group, GDK_KEY_P, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(ApplyBTN, "activate", accel_group, GDK_KEY_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	//gtk_widget_add_accelerator(submenu_item2, "activate", accel_group, GDK_KEY_R, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(window);

	if (!is_user_root())
	{
		show_error_dialog(window);
		gtk_widget_destroy(window);
		return 1;
	}

	gtk_main();
	g_ptr_array_free(group_array, TRUE);
	return 0;
}
