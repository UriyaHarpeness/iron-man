import traceback
from cmd import Cmd

import pathlib

from iron_man import IronMan


class IronManShell(Cmd):
    intro = 'Welcome to Iron Man\'s shell.   Type help or ? to list commands.\n'
    prompt = '(iron man) '

    def __init__(self):
        super().__init__()
        self.im = None

    def onecmd(self, line):
        """Interpret the argument as though it had been typed in response
        to the prompt.

        This may be overridden, but should not normally need to be;
        see the precmd() and postcmd() methods for useful execution hooks.
        The return value is a flag indicating whether interpretation of
        commands by the interpreter should stop.

        """
        cmd, arg, line = self.parseline(line)
        if not line:
            return self.emptyline()
        if cmd is None:
            return self.default(line)
        self.lastcmd = line
        if line == 'EOF':
            self.lastcmd = ''
        if cmd == '':
            return self.default(line)
        else:
            try:
                func = getattr(self, 'do_' + cmd)
            except AttributeError:
                return self.default(line)

            if f'do_{cmd}' not in self.get_names():
                print(f'Command "{cmd}" is not supported at this state.')
                return

            try:
                return func(arg)
            except Exception as e:
                traceback.print_tb(e.__traceback__)
                print(e)

    def get_names(self):
        return self.GENERAL_COMMANDS + (self.CONNECTED_COMMANDS if self.im else self.DISCONNECTED_COMMANDS)

    def do_connect(self, arg):
        """
        Connect to an Iron Man server with given config path.
        """
        self.im = IronMan(pathlib.Path(arg))

    def do_get_file(self, arg: str):
        """
        Get a file by path.
        """
        print(self.im.get_file(arg))

    def do_put_file(self, arg: str):
        """
        Put a file by path and content.
        """
        self.im.put_file(arg[:arg.find(' ')], arg[arg.find(' ') + 1:])

    def do_run_shell(self, arg: str):
        """
        Run a command with optional arguments.
        """
        if arg.find(' ') == -1:
            args = [arg]
        else:
            args = [arg[:arg.find(' ')], arg[arg.find(' ') + 1:].split(' ')]
        print('Exit code:', self.im.run_shell(*args))

    def do_disconnect(self, _=None):
        """
        Disconnect from an Iron Man server.
        """
        self.im = None

    def do_exit(self, _=None):
        """
        Exit the Iron Man shell.
        """
        self.do_disconnect()
        return True

    def __del__(self):
        self.do_exit()

    DISCONNECTED_COMMANDS = ['do_connect']
    CONNECTED_COMMANDS = ['do_get_file',
                          'do_put_file',
                          'do_run_shell',
                          'do_disconnect']
    GENERAL_COMMANDS = ['do_help',
                        'do_exit']


if __name__ == '__main__':
    IronManShell().cmdloop()
