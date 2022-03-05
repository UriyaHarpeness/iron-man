SECTIONS
{
  /**
   * Add the builtin commands' code to the .text section, and save their start and end addresses.
   */
  .text : {
    *(.text)

    /* Get file command's section. */
    __get_file_start = .;
    *(.get_file)
    __get_file_end = .;

    /* Put file command's section. */
    __put_file_start = .;
    *(.put_file)
    __put_file_end = .;

    /* Run shell command's section. */
    __run_shell_start = .;
    *(.run_shell)
    __run_shell_end = .;
  }
}
INSERT AFTER .plt.got;
