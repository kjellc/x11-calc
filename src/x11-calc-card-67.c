/*
 * x11-calc-card.c - RPN (Reverse Polish) calculator simulator.
 *
 * Copyright(C) 2024  Kjellc
 *
 * Implements the card reader for HP67
 * The file chooser part uses the GTK libraries
 */

#include <gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "x11-calc-label.h"
#include "x11-calc-switch.h"
#include "x11-calc-button.h"
#include "x11-calc-cpu.h"

#include "x11-calc-card-67.h"

#define NON_BINARY 1

#if defined(HP67)  /* only hp67 has a card reader */

/*
** called when the processor executes instruction 0x3c0 [read from or write to card]
** read or write to the open file depending on the flag crc[WRITE]
 */
void v_card_read_write_record(oprocessor* h_processor)
{
    if (h_processor->card_file == NULL)
        return;  /* no file open, do nothing */

    if (h_processor->crc[BUFFER] == False)
        return;  /* buffer is not ready, do nothing */

    /* write a card-record if the buffer addr == 0x99 (std HP67) or 0xF9 (HP67+) */
    if (h_processor->crc[WRITE] &&
        (h_processor->addr == 0x99 || h_processor->addr == 0xF9))
    {
        /* write the 7 most significant nibbles of reg 99/f9 to file */
        unsigned char* nibblep = &(h_processor->mem[h_processor->addr]->nibble[REG_SIZE - 1]);

#if NON_BINARY
        int record = 0;
        for (int i = 0; i < 7; i++) {
            record <<= 4;
            record += *nibblep--;
        }
        fprintf(h_processor->card_file, "%07x,", record);
        if (h_processor->card_records % 8 == 0)
            fprintf(h_processor->card_file, "\n");

        /* debug */
        if (h_processor->card_records == 0)
            fprintf(stdout, "\nWrite to card-file:\n");
        fprintf(stdout, "%07x ", record);
        if (h_processor->card_records % 8 == 0)
            fprintf(stdout, "\n");

#else // Binary (obsolete)
        unsigned char buf[4];
        buf[0] = *nibblep-- << 4;
        buf[0] += *nibblep--;
        buf[1] = *nibblep-- << 4;
        buf[1] += *nibblep--;
        buf[2] = *nibblep-- << 4;
        buf[2] += *nibblep--;
        buf[3] = *nibblep << 4;
        fwrite(buf, 1, 4, h_processor->card_file);
#endif
        /* fprintf(stdout, "Write: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]); */
        h_processor->card_records++;
    }

    /* read a card-record if the buffer addr == 0x99 or 0x9b (std HP67) or 0xf9 or 0xfb (HP67+) */
    else if (h_processor->crc[WRITE] == False &&
               (h_processor->addr == 0x99 || h_processor->addr == 0x9b /*std*/ ||
                h_processor->addr == 0xF9 || h_processor->addr == 0xFb /*67+*/))
    {
        /* read 7 nibbles from file to the most and least (duplicated) significant nibbles of reg 9b/fb */
        int reg_ptr = (h_processor->addr & 0xF0) + 0x0B;  /* 0x9B or 0xFB */
        unsigned char* nibblep = &(h_processor->mem[reg_ptr]->nibble[REG_SIZE - 1]);

#if NON_BINARY
        int record = 0;
        fscanf(h_processor->card_file, "%x,", (unsigned int*)&record);
        /* put record in upper 7 nibbles */
        for (int i = 6; i >= 0; i--) {
            *nibblep-- = (record >> i*4) & 0xf;
        }
        /* put record in lower 7 nibbles as well! */
        for (int i = 6; i >= 0; i--) {
            *nibblep-- = (record >> i*4) & 0xf;
        }

        /* debug */
        if (h_processor->card_records == 0)
            fprintf(stdout, "\nRead from card-file:\n");
        fprintf(stdout, "%07x ", record);
        if (h_processor->card_records % 8 == 0)
            fprintf(stdout, "\n");

#else // Binary (obsolete)
        unsigned char buf[4];
        fread(buf, 1, 4, h_processor->card_file);
        /* put record in upper nibbles */
        *nibblep-- = buf[0] >> 4;
        *nibblep-- = buf[0] & 0xf;
        *nibblep-- = buf[1] >> 4;
        *nibblep-- = buf[1] & 0xf;
        *nibblep-- = buf[2] >> 4;
        *nibblep-- = buf[2] & 0xf;
        *nibblep-- = buf[3] >> 4;
        /* put record in lower nibbles as well! */
        *nibblep-- = buf[0] >> 4;
        *nibblep-- = buf[0] & 0xf;
        *nibblep-- = buf[1] >> 4;
        *nibblep-- = buf[1] & 0xf;
        *nibblep-- = buf[2] >> 4;
        *nibblep-- = buf[2] & 0xf;
        *nibblep-- = buf[3] >> 4;
        fprintf(stdout, "Read: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
#endif
        h_processor->card_records++;
        // moved to cpu.c
        //h_processor->crc[BUFFER] = False;  /* buffer empty now, must test before next r/w */
    }
}


/*
** Open a file load/save dialog
**
 */
char *FileChooser(char* aTitle,       // window title
                  int aSave,          /* 1: save dialog, 0: load */
                  int aHomeDir)       /* 1: start in home dir, 0: default dir */
{
  GtkWidget *lDialog;
  char *lFilename = NULL;

  int argc = 0;
  char** argv;
  gtk_init(&argc, &argv);

  /* Create a file chooser dialog */
  lDialog = gtk_file_chooser_dialog_new(aTitle, NULL /* window */,
                                        aSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "_Cancel", GTK_RESPONSE_CANCEL,                 /* cancel */
                                        aSave ? "_Save" : "_Load", GTK_RESPONSE_ACCEPT, /* save/load */
                                        NULL);

  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (lDialog), TRUE);

  if (aHomeDir)
  {
      /* Set the default location (optional) */
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (lDialog), g_get_home_dir());
  }

  GtkFileFilter* filter67 = gtk_file_filter_new();
  gtk_file_filter_set_name(filter67, aSave ? "HP67 Save Files (*.hp67)" : "HP67 Load Files (*.hp67)");
  gtk_file_filter_add_pattern(filter67, "*.hp67");

  GtkFileFilter* filterAll = gtk_file_filter_new();
  gtk_file_filter_set_name(filterAll, "All Files (*.*)");
  gtk_file_filter_add_pattern (filterAll, "*.*");

  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(lDialog), filter67);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(lDialog), filterAll);

  /* Show the dialog and wait for user response */
  if (gtk_dialog_run (GTK_DIALOG(lDialog)) == GTK_RESPONSE_ACCEPT)
  {
    /* Get the selected filename */
    lFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (lDialog));
  }

  /* Destroy the dialog */
  gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(lDialog), filter67);
  gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(lDialog), filterAll);

  gtk_widget_destroy (lDialog);

  /* service pending events - instead of calling gtk_main() */
  while ( gtk_events_pending() ) gtk_main_iteration();

  return lFilename;
}


/*
** Open the file chooser dialow window
** if h_processor->crc[WRITE] = True, the user is asked for a save file
** else the user is asked for a read file
*/
void v_card_open_file(oprocessor* h_processor)
{
    h_processor->card_file = NULL;
    h_processor->card_records = 0;

    if (h_processor->crc[WRITE]) {
        fprintf(stdout, "\nMotor on, open card-save file...\n");
        char* filename = FileChooser("Save", 1, 1);
        fprintf(stdout, "file=%s\n", (filename ? filename : "CANCELED"));
        if (filename)
        {
            h_processor->card_file = fopen(filename, "w");
            free(filename);  /* Free the allocated memory */
        }
    } else {
        fprintf(stdout, "\nMotor on, open card-load file...\n");
        char* filename = FileChooser("Load", 0, 1);
        fprintf(stdout, "file=%s\n", (filename ? filename : "CANCELED"));
        if (filename)
        {
            h_processor->card_file = fopen(filename, "r");
            free(filename);  /* Free the allocated memory */
        }
    }

    h_processor->crc[CARD] = False ;  /* kjc: Card removed immediately after starting */
    // moved to cpu.c
    //h_processor->flags[DISPLAY_ENABLE] = True;  /* enable again */
}

void v_card_close_file(oprocessor* h_processor)
{
    h_processor->crc[CARD] = False;    /* kjc: card removed when motor is stopped ??? */

    if (h_processor->card_file) {

        if (h_processor->crc[WRITE])
            fprintf(h_processor->card_file, "\n"); /* end file with newline */

        fprintf(stdout, "\n");                     /* end debug with newline */
        fclose(h_processor->card_file);
    }
    h_processor->card_file = NULL;
}

#endif // HP67
