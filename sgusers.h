#include <gtk/gtk.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <glib.h>
#include <ctype.h>
char *pm;

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
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
	char groups_buffer[4096];
	snprintf(groups_buffer, sizeof(groups_buffer), "groups %s", old_username);
	FILE *groups_pipe = popen(groups_buffer, "r");
	if (groups_pipe) 
	{
		fgets(groups_buffer, sizeof(groups_buffer), groups_pipe);
		pclose(groups_pipe);
		groups_buffer[strcspn(groups_buffer, "\n")] = '\0';
		gtk_entry_set_text(GTK_ENTRY(entry_groups), groups_buffer);
	}

	gtk_widget_show_all(dialog);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		const char *new_username = gtk_entry_get_text(GTK_ENTRY(entry));
		if (strcmp(new_username, "") != 0)
		{

			char groupquery[4096];

			snprintf(groupquery, sizeof(groupquery), "groups %s", old_username);
			FILE *group_pipe = popen(groupquery, "r");
			if (group_pipe == NULL) 
			{
				perror("Failed to fetch user groups");
			}

			char oldusergroups[4096];
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

			char remove_from_all_groups_command[4096];
			snprintf(remove_from_all_groups_command, sizeof(remove_from_all_groups_command), "usermod -r -G %s %s", oldusergroups, old_username);
				system(remove_from_all_groups_command);
				printf("%s\n", remove_from_all_groups_command);
	
			// Rename the user
			char rename_command[4096];
			snprintf(rename_command, sizeof(rename_command), "usermod -l %s %s", new_username, old_username);
				system(rename_command);
				printf("%s\n", rename_command);

			// Get the new groups from the entry
			const char *new_groups = gtk_entry_get_text(GTK_ENTRY(entry_groups));

			// Split the group names and add the user to each group
			char *group = strtok((char *)new_groups, " ,");
			while (group != NULL)
			{
				char add_to_group_command[4096];
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
			char command[4096];
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

		gint password_response = gtk_dialog_run(GTK_DIALOG(password_dialog));
		if (password_response == GTK_RESPONSE_ACCEPT)
		{
			const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(password_entry));
			gchar command[4096];
			snprintf(command, sizeof(command), "echo '%s:%s' | chpasswd", old_username, new_password);
			system(command);
			printf("%s\n", command);
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

	gtk_widget_show_all(dialog);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

if (response == GTK_RESPONSE_ACCEPT)
{
	const char *new_username = gtk_entry_get_text(GTK_ENTRY(entry_name));
	const char *new_groups = gtk_entry_get_text(GTK_ENTRY(entry_groups));
	const char *new_passwd = gtk_entry_get_text(GTK_ENTRY(entry_passwd));
	char modified_groups[4096];
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
	modified_groups[j] = '\0'; // Null-terminate the modified_groups string

	if (strcmp(new_username, "") != 0)
	{
		char command[4096];
		if (strcmp(modified_groups, "") != 0)
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
			printf("%s\n", command);
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

	char line[1024];
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
	gtk_widget_show_all(rdialog);

	gint password_response = gtk_dialog_run(GTK_DIALOG(rdialog));
	if (password_response == GTK_RESPONSE_ACCEPT)
	{
		const gchar *new_password = gtk_entry_get_text(GTK_ENTRY(password_entry));
		gchar command[4096];
		snprintf(command, sizeof(command), "echo 'root:%s' | chpasswd", new_password);
		system(command);
		printf("%s\n", command);
	}

	gtk_widget_destroy(rdialog);
}
