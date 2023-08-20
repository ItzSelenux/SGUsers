#include <gtk/gtk.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <glib.h>
#include <ctype.h>
#define ML 1024
char *argvusername[ML];
char *pm;
char selected_photo_path[ML] = "";



typedef struct
{
	char fname[ML];
	char office[ML];
	char phone[ML];
	char officephone[ML];
	char email[ML];
}UserData;




static gboolean is_valid_char_ws(gunichar ch) {
    return (g_unichar_isalnum(ch) || ch == '_') && (g_unichar_islower(ch));
}

static void on_entry_changed_ws(GtkEditable *editable, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
    gchar *new_text = g_strdup(text);
    gint i, j;
    gboolean last_char_space = FALSE;

    for (i = 0, j = 0; text[i]; i++) {
        if (is_valid_char_ws(text[i])) {
            new_text[j] = text[i];
            last_char_space = FALSE;
            j++;
        } else if (!last_char_space) {
            new_text[j] = ' ';
            last_char_space = TRUE;
            j++;
        }
    }

    new_text[j] = '\0';

    if (strcmp(text, new_text) != 0) {
        gtk_entry_set_text(GTK_ENTRY(editable), new_text);
    }

    g_free(new_text);
}


static gboolean is_valid_char(gunichar ch) {
	return (g_unichar_isalnum(ch) || ch == '_') && (g_unichar_islower(ch));
}

static void on_entry_changed(GtkEditable *editable, gpointer user_data) {
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
	GString *new_text = g_string_new("");
	for (gint i = 0; text[i]; i++)
	{
		gunichar ch = g_utf8_get_char(&text[i]);
		if (is_valid_char(ch))
		{
			new_text = g_string_append_unichar(new_text, ch);
		}
	}
	if (strcmp(text, new_text->str) != 0)
	{
		gtk_entry_set_text(GTK_ENTRY(editable), new_text->str);
	}
	g_string_free(new_text, TRUE);
}

static gboolean is_valid_char_passwd(gunichar ch) {
	return (g_unichar_isalnum(ch) || ch == '_') && (g_unichar_islower(ch) || g_unichar_isdigit(ch));
}

static void on_entry_changed_passwd(GtkEditable *editable, gpointer user_data) {
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
	GString *new_text = g_string_new("");
	for (gint i = 0; text[i]; i++)
	{
		gunichar ch = g_utf8_get_char(&text[i]);
		if (is_valid_char_passwd(ch))
		{
			new_text = g_string_append_unichar(new_text, ch);
		}
	}
	if (strcmp(text, new_text->str) != 0)
	{
		gtk_entry_set_text(GTK_ENTRY(editable), new_text->str);
	}
	g_string_free(new_text, TRUE);
}

void uie_savedata(GtkWidget *button, gpointer data)
{
	// Cast the data pointer to the correct types
	GtkWidget *fname_entry = (GtkWidget *)data;
	GtkWidget *phone_entry = (GtkWidget *)g_object_get_data(G_OBJECT(fname_entry), "phone_entry");
	GtkWidget *email_entry = (GtkWidget *)g_object_get_data(G_OBJECT(fname_entry), "email_entry");
	GtkWidget *office_entry = (GtkWidget *)g_object_get_data(G_OBJECT(fname_entry), "office_entry");
	GtkWidget *ofn_entry = (GtkWidget *)g_object_get_data(G_OBJECT(fname_entry), "ofn_entry");
	// Get the text from the entries
	const char *fname = gtk_entry_get_text(GTK_ENTRY(fname_entry));
	const char *phone = gtk_entry_get_text(GTK_ENTRY(phone_entry));
	const char *email = gtk_entry_get_text(GTK_ENTRY(email_entry));
	const char *office = gtk_entry_get_text(GTK_ENTRY(office_entry));
	const char *ofn = gtk_entry_get_text(GTK_ENTRY(ofn_entry));

	char uie_savecmd[ML];

	if (strlen(selected_photo_path) == 0)
	{
	printf("profile picture not changed\n");
	}
	else 
	{
		  // Copy the file to /home/*argvuser/.face
		char destination[256];
		sprintf(destination, "/home/%s/.face", *argvusername); // Assuming argv[1] contains the username

		FILE *src_file, *dest_file;
		src_file = fopen(selected_photo_path, "rb");
		dest_file = fopen(destination, "wb");

		if (src_file == NULL || dest_file == NULL)
		{
			printf("Error opening files.\n");
		}

		char buffer[1024];
		size_t bytesRead;

		while ((bytesRead = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
		{
			fwrite(buffer, 1, bytesRead, dest_file);
		}

		fclose(src_file);
		fclose(dest_file);

		printf("File copied successfully to %s\n", destination);
	}

	snprintf(uie_savecmd, sizeof(uie_savecmd), "usermod -c \"%s,%s,%s,%s,%s\" %s", fname, phone, email, office, ofn, *argvusername);
	g_print("\n");

	system(uie_savecmd);
	printf("%s\n", uie_savecmd);

	g_print("First Name: %s\n", fname);
	g_print("Phone: %s\n", phone);
	g_print("Email: %s\n", email);
	g_print("Office: %s\n", office);
	g_print("Office Number: %s\n", ofn);
	
}


void extractUserData(UserData* data, const char* username)
{
	char command[ML];
	FILE* pipe;
	char buffer[ML];

	sprintf(command, "grep -e '%s' /etc/passwd | cut -d ':' -f 5", username);

	pipe = popen(command, "r");
	if (pipe == NULL)
	{
		printf("Error executing command.\n");
		return;
	}

	fgets(buffer, sizeof(buffer), pipe);
	pclose(pipe);

	char* token;
	int field = 1;
	token = strtok(buffer, ",");

	while (token != NULL)
	{
		size_t token_len = strcspn(token, "\r\n"); 
		token[token_len] = '\0';

		switch (field)
		{
			case 1:
				strcpy(data->fname, token);
				break;
			case 2:
				strcpy(data->office, token);
				break;
			case 3:
				strcpy(data->phone, token);
				break;
			case 4:
				strcpy(data->officephone, token);
				break;
			case 5:
				strcpy(data->email, token);
				break;
			default:
				break;
		}

		token = strtok(NULL, ",");
		field++;
	}
}

void get_username(char** argvusername)
{
	FILE *fp;
	char buffer[128];
	fp = popen("id 1000 | cut -d'(' -f2 | cut -d')' -f1", "r");
	if (fp == NULL)
	{
		printf("Error while trying to catch user, you need to specify a user in the argv.\n");
		return;
	}
	if (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n')
		{
			buffer[len - 1] = '\0';
		}
		*argvusername = (char*)malloc(strlen(buffer) + 1);
		strcpy(*argvusername, buffer);
	}
	pclose(fp);
}

void on_pfimage_clicked(GtkButton *button, gpointer data)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Select an image",
													GTK_WINDOW(data),
													GTK_FILE_CHOOSER_ACTION_OPEN,
													"_Cancel",
													GTK_RESPONSE_CANCEL,
													"_Open",
													GTK_RESPONSE_ACCEPT,
													NULL);


	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_mime_type(filter, "image/*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT)
	{

		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

 
		strncpy(selected_photo_path, filename, sizeof(selected_photo_path) - 1);
		selected_photo_path[sizeof(selected_photo_path) - 1] = '\0';

		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(selected_photo_path, NULL);

		GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);

		GtkWidget *image = gtk_image_new_from_pixbuf(resized_pixbuf);
		gtk_button_set_image(GTK_BUTTON(data), image);
	g_object_unref(resized_pixbuf);
	g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		GtkWidget *submenu = GTK_WIDGET(data);
		gtk_menu_popup_at_pointer(GTK_MENU(submenu), NULL);
		return TRUE;
	}

	return FALSE;
}

gboolean is_user_root()
{
	return (geteuid() == 0);
}

void show_error_dialog(GtkWidget *parent)
{
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
											  GTK_DIALOG_MODAL,
											  GTK_MESSAGE_ERROR,
											  GTK_BUTTONS_OK,
											  "Error: You need to be root to use this program.");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void restart_program(GtkWidget *widget, gpointer data)
{
	printf("Program Reloaded...\n");
	char *args[] = {pm, NULL};
	execvp(args[0], args);
}


void uie(GtkWidget *widget, gpointer data)
{
	const char *old_username = (const char *)data;
	char* path1 = "/usr/bin/sgusers-uie";
	char* path2 = "./sgusers-uie";
	char command[100];

	snprintf(command, sizeof(command), "%s %s", "/usr/bin/sgusers-uie", old_username);
	
	if (access(command, X_OK) == 0)
	{
		printf("executing %s\n", command);
		system(command);
	}
	else if (access("sgusers-uie", X_OK) == 0)
	{
		snprintf(command, sizeof(command), "%s %s", "./sgusers-uie", old_username);
		printf("executing %s\n", command);
		system(command);
	}
	else 
	{
		printf("Error: can't load sgusers-uie, please reinstall this program\n");
	}
}



void edit_user(GtkWidget *widget, gpointer data)
{
	const char *old_username = (const char *)data;

	GtkWidget *dialog;
	dialog = gtk_dialog_new_with_buttons("Edit user - SGUsers",
										 GTK_WINDOW(gtk_widget_get_toplevel(widget)),
										 GTK_DIALOG_MODAL,
										 "Save",
										 GTK_RESPONSE_ACCEPT,
										 "Change Password",
										 GTK_RESPONSE_YES,
										 "Delete User",
										 GTK_RESPONSE_NO,
										 "Cancel",
										 GTK_RESPONSE_CANCEL,
										 NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "cs-user", 48, 0);
	if (info != NULL)
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}


	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	GtkWidget *label0 = gtk_label_new("Username:");
	gtk_container_add(GTK_CONTAINER(content_area), label0);

	GtkWidget *entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), old_username);
	gtk_container_add(GTK_CONTAINER(content_area), entry);

	GtkWidget *label_groups = gtk_label_new("Groups:");
	gtk_container_add(GTK_CONTAINER(content_area), label_groups);

	GtkWidget *entry_groups = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(content_area), entry_groups);

	// Get the current user's groups and set them in the entry
	char groups_buffer[ML];
	snprintf(groups_buffer, sizeof(groups_buffer), "groups %s", old_username);
	FILE *groups_pipe = popen(groups_buffer, "r");
	if (groups_pipe) 
	{
		fgets(groups_buffer, sizeof(groups_buffer), groups_pipe);
		pclose(groups_pipe);
		groups_buffer[strcspn(groups_buffer, "\n")] = '\0';
		gtk_entry_set_text(GTK_ENTRY(entry_groups), groups_buffer);
	}
	g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);
	g_signal_connect(entry_groups, "changed", G_CALLBACK(on_entry_changed_ws), NULL);
	gtk_widget_show_all(dialog);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		const char *new_username = gtk_entry_get_text(GTK_ENTRY(entry));
		if (strcmp(new_username, "") != 0)
		{

			char groupquery[ML];

			snprintf(groupquery, sizeof(groupquery), "groups %s", old_username);
			FILE *group_pipe = popen(groupquery, "r");
			if (group_pipe == NULL) 
			{
				perror("Failed to fetch user groups");
			}

			char oldusergroups[ML];
			fgets(oldusergroups, sizeof(oldusergroups), group_pipe);
			pclose(group_pipe);

			//remove trailing space in variable
			size_t len = strlen(oldusergroups);
			while (len > 0 && isspace(oldusergroups[len - 1])) 
			{
				oldusergroups[--len] = '\0';
			}

			char* p = oldusergroups;
			while (*p != '\0') 
			{
				if (*p == ' ') 
				{
					*p = ','; 
				}
				p++;
			}

			char remove_from_all_groups_command[ML];
			snprintf(remove_from_all_groups_command, sizeof(remove_from_all_groups_command), "usermod -r -G %s %s", oldusergroups, old_username);
				system(remove_from_all_groups_command);
				printf("%s\n", remove_from_all_groups_command);
	
			// Rename the user
			char rename_command[ML];
			snprintf(rename_command, sizeof(rename_command), "usermod -l %s %s", new_username, old_username);
				system(rename_command);
				printf("%s\n", rename_command);

			// Get the new groups from the entry
			const char *new_groups = gtk_entry_get_text(GTK_ENTRY(entry_groups));

			// Split the group names and add the user to each group
			char *group = strtok((char *)new_groups, " ,");
			while (group != NULL)
			{
				char add_to_group_command[ML];
				snprintf(add_to_group_command, sizeof(add_to_group_command), "usermod -a -G %s %s", group, new_username);
				system(add_to_group_command);
					printf("%s\n", add_to_group_command);
					group = strtok(NULL, " ,");
			}

			restart_program(NULL, pm);
		}
	}
	else if (response == GTK_RESPONSE_NO)
	{
	GtkWidget *confirm_dialog;
		confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
												GTK_DIALOG_MODAL,
												GTK_MESSAGE_QUESTION,
												GTK_BUTTONS_YES_NO,
												"Are you sure you want to delete the user?");
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "list-remove-user", 48, 0);
	gtk_window_set_position(GTK_WINDOW(confirm_dialog), GTK_WIN_POS_CENTER);
	if (info != NULL)
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(confirm_dialog), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}
		gint confirm_response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
		gtk_widget_destroy(confirm_dialog);

		if (confirm_response == GTK_RESPONSE_YES)
		{
			char command[ML];
			snprintf(command, sizeof(command), "userdel %s", old_username);
			system(command);
			printf("%s\n", command);
			restart_program(NULL, pm);
		}
	}

	else if (response == GTK_RESPONSE_YES)
	{
		GtkWidget *password_dialog;
		password_dialog = gtk_dialog_new_with_buttons("Change Password",
													GTK_WINDOW(dialog),
													GTK_DIALOG_MODAL,
													"Change", GTK_RESPONSE_ACCEPT,
													"Cancel", GTK_RESPONSE_CANCEL,
													NULL);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "dialog-password", 48, 0);
	gtk_window_set_position(GTK_WINDOW(password_dialog), GTK_WIN_POS_CENTER);
	if (info != NULL)
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(password_dialog), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}
		GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(password_dialog));
		GtkWidget *password_label = gtk_label_new("Enter new password (left empty to disable):");
		GtkWidget *password_entry = gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
		gtk_container_add(GTK_CONTAINER(content_area), password_label);
		gtk_container_add(GTK_CONTAINER(content_area), password_entry);
		gtk_widget_show_all(password_dialog);
		g_signal_connect(password_entry, "changed", G_CALLBACK(on_entry_changed_passwd), NULL);
		gint password_response = gtk_dialog_run(GTK_DIALOG(password_dialog));
		if (password_response == GTK_RESPONSE_ACCEPT)
		{
			GtkWidget *confirm_dialog;
	confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
										   GTK_DIALOG_MODAL,
										   GTK_MESSAGE_QUESTION,
										   GTK_BUTTONS_YES_NO,
										   "Are you sure you want to change the password?");
	gtk_window_set_position(GTK_WINDOW(confirm_dialog), GTK_WIN_POS_CENTER);

	gint confirm_response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
	gtk_widget_destroy(confirm_dialog);

	if (confirm_response == GTK_RESPONSE_YES)
	{
			const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(password_entry));
			gchar command[ML];
			snprintf(command, sizeof(command), "echo '%s:%s' | chpasswd", old_username, new_password);
			system(command);
		}
		}
		gtk_widget_destroy(password_dialog);
	}
	gtk_widget_destroy(dialog);
}


void add_user(GtkWidget *widget, gpointer data) 
{
	GtkWidget *dialog;
	dialog = gtk_dialog_new_with_buttons("Add new user",
										 GTK_WINDOW(gtk_widget_get_toplevel(widget)),
										 GTK_DIALOG_MODAL,
										 "OK",
										 GTK_RESPONSE_ACCEPT,
										 "Cancel",
										 GTK_RESPONSE_CANCEL,
										 NULL);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "list-add-user", 48, 0);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	if (info != NULL)
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(dialog), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}
	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	GtkWidget *label_name = gtk_label_new("Username:");
	gtk_container_add(GTK_CONTAINER(content_area), label_name);


	GtkWidget *entry_name = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(content_area), entry_name);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Selene");

	GtkWidget *label_groups = gtk_label_new("Groups:");
	gtk_container_add(GTK_CONTAINER(content_area), label_groups);

	GtkWidget *entry_groups = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(content_area), entry_groups);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry_groups), "wheel adm sftp");

	GtkWidget *label_passwd = gtk_label_new("Password:");
	gtk_container_add(GTK_CONTAINER(content_area), label_passwd);

	GtkWidget *entry_passwd = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(content_area), entry_passwd);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry_passwd), "Password");
	gtk_entry_set_visibility(GTK_ENTRY(entry_passwd), FALSE);
	g_signal_connect(entry_passwd, "changed", G_CALLBACK(on_entry_changed_passwd), NULL);
	g_signal_connect(entry_name, "changed", G_CALLBACK(on_entry_changed), NULL);
	g_signal_connect(entry_groups, "changed", G_CALLBACK(on_entry_changed_ws), NULL);
	gtk_widget_show_all(dialog);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

if (response == GTK_RESPONSE_ACCEPT)
{
	const char *new_username = gtk_entry_get_text(GTK_ENTRY(entry_name));
	const char *new_groups = gtk_entry_get_text(GTK_ENTRY(entry_groups));
	const char *new_passwd = gtk_entry_get_text(GTK_ENTRY(entry_passwd));
	char modified_groups[ML];
	size_t len = strlen(new_groups);
	size_t j = 0;

	for (size_t i = 0; i < len; i++)
	{
		if (new_groups[i] == ' ')
		{
			modified_groups[j++] = ',';
		}
		else
		{
			modified_groups[j++] = new_groups[i];
		}
	}
	modified_groups[j] = '\0';

	if (strcmp(new_username, "") != 0)
	{
		char command[ML];
		if (strcmp(modified_groups, "") == 0 || strcmp(modified_groups, " ") == 0)
		{
			snprintf(command, sizeof(command), "useradd -m -G %s %s", modified_groups, new_username);
		}
		else
		{
			snprintf(command, sizeof(command), "useradd -m %s", new_username);
		}
		system(command);
		printf("%s\n", command);


		if (strcmp(new_passwd, "") != 0)
		{
			snprintf(command, sizeof(command), "echo '%s:%s' | chpasswd", new_username, new_passwd);
			system(command);
		}
		else
		{
			printf("No password provided\n");
		}


		restart_program(NULL, pm);
	}
	

}

gtk_widget_destroy(dialog);
}



void on_submenu_item3_selected(GtkMenuItem *menuitem, gpointer userdata) 
{
	GtkWidget *dialog;
	dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "SGUsers");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Copyright © 2023 ItzSelenux for Simple GTK Desktop Environment");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "SGDE User Management Program");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://sgde.github.io/sgusers");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), "Project WebSite");
	gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog),GTK_LICENSE_GPL_3_0);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog),"cs-user");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

typedef struct 
{
	gchar *group_name;
	gchar *user_list;
}
GroupInfo;


gboolean read_group_info(GPtrArray *group_array)
{
	FILE *group_file = fopen("/etc/group", "r");
	if (!group_file)
		{
			g_error("Failed to open /etc/group");
			return FALSE;
		}

	char line[ML];
	while (fgets(line, sizeof(line), group_file))
		{
			gchar **tokens = g_strsplit(line, ":", -1);
			if (tokens[0] && tokens[3])
			{
				GroupInfo *group_info = g_new(GroupInfo, 1);
				group_info->group_name = g_strdup(tokens[0]);
				group_info->user_list = g_strdup(tokens[3]);
				g_ptr_array_add(group_array, group_info);
			}
			g_strfreev(tokens);
		}
	fclose(group_file);
	return TRUE;
}

// create database
void free_group_info(gpointer data) 
{
	GroupInfo *group_info = (GroupInfo *)data;
	g_free(group_info->group_name);
	g_free(group_info->user_list);
	g_free(group_info);
}


void on_submenu_item1_selected(GtkWidget *widget, gpointer data)
{
	GPtrArray *group_array = (GPtrArray *)data;
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "System's Groups - SGrandr");
	gtk_window_set_default_size(GTK_WINDOW(window), 222, 444);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "user-group-properties", 48, 0);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	if (info != NULL) 
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(window), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}
	GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
								GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *text_view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));


	//concatenate data
	GString *group_info_str = g_string_new("");
	for (int i = 0; i < group_array->len; i++) 
	{
		GroupInfo *group_info = g_ptr_array_index(group_array, i);
		g_string_append_printf(group_info_str, "%s\nUsers: %s\n\n", group_info->group_name, group_info->user_list);
	}

	gtk_text_buffer_set_text(buffer, group_info_str->str, -1);
	g_string_free(group_info_str, TRUE);

	gtk_container_add(GTK_CONTAINER(scroll_window), text_view);
	gtk_container_add(GTK_CONTAINER(window), scroll_window);

	gtk_widget_show_all(window);
}


void on_submenu_item4_selected(GtkWidget *widget, gpointer data)
{
	GtkWidget *rdialog;
	rdialog = gtk_dialog_new_with_buttons("Root Account",
										GTK_WINDOW(gtk_widget_get_toplevel(widget)),
										GTK_DIALOG_MODAL,
										"Change", GTK_RESPONSE_ACCEPT,
										"Cancel", GTK_RESPONSE_CANCEL,
										NULL);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GtkIconInfo *info = gtk_icon_theme_lookup_icon(theme, "dialog-password", 48, 0);
	gtk_window_set_position(GTK_WINDOW(rdialog), GTK_WIN_POS_CENTER);
	if (info != NULL)
	{
		GdkPixbuf *icon = gtk_icon_info_load_icon(info, NULL);
		gtk_window_set_icon(GTK_WINDOW(rdialog), icon);
		g_object_unref(icon);
		g_object_unref(info);
	}

	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(rdialog));
	GtkWidget *password_label = gtk_label_new("Change root password (left empty to disable):");
	GtkWidget *password_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
	gtk_container_add(GTK_CONTAINER(content_area), password_label);
	gtk_container_add(GTK_CONTAINER(content_area), password_entry);
	g_signal_connect(password_entry, "changed", G_CALLBACK(on_entry_changed_passwd), NULL);
	gtk_widget_show_all(rdialog);

	gint password_response = gtk_dialog_run(GTK_DIALOG(rdialog));
	if (password_response == GTK_RESPONSE_ACCEPT)
	{
		// Show confirmation dialog
		GtkWidget *confirmation_dialog = gtk_message_dialog_new(GTK_WINDOW(rdialog),
															  GTK_DIALOG_MODAL,
															  GTK_MESSAGE_QUESTION,
															  GTK_BUTTONS_OK_CANCEL,
															  "Are you sure you want to change the root password?");
		gint confirmation_response = gtk_dialog_run(GTK_DIALOG(confirmation_dialog));
		gtk_widget_destroy(confirmation_dialog);

		if (confirmation_response == GTK_RESPONSE_OK)
		{
			const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(password_entry));
			gchar command[ML];
			snprintf(command, sizeof(command), "echo 'root:%s' | chpasswd", new_password);
			system(command);
		}
	}

	gtk_widget_destroy(rdialog);
}
