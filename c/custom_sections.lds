SECTIONS
{
  .text : {
    *(.text)

    __get_file_start = .;
    *(.get_file)
    __get_file_end = .;

    __put_file_start = .;
    *(.put_file)
    __put_file_end = .;

    __run_shell_start = .;
    *(.run_shell)
    __run_shell_end = .;
  }
}
INSERT AFTER .plt.got;
