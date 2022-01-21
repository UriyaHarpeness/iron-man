import traceback
from cmd import Cmd

import pathlib

from iron_man import IronMan
from struct_conversion import convert_to_types_by_format


# todo: parse spaces, tabs, and ""s.
class IronManShell(Cmd):
    intro = 'Welcome to Iron Man\'s shell.   Type help or ? to list commands.\n'
    prompt = '(iron man) '

    def __init__(self):
        super().__init__()
        self.im = None

    def onecmd(self, line):
        """
        Interpret the argument as though it had been typed in response
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
                if self.im and cmd in self.im.module_commands:
                    return self.run_module_command(cmd, arg)
                return self.default(line)

            if f'do_{cmd}' not in self.get_names():
                print(f'Command "{cmd}" is not supported at this state.')
                return

            try:
                return func(arg)
            except Exception as e:
                traceback.print_tb(e.__traceback__)
                print(e)

    def do_help(self, arg):
        """
        List available commands with "help" or detailed help with "help cmd".
        """
        if arg:
            # XXX check arg syntax
            try:
                func = getattr(self, 'help_' + arg)
            except AttributeError:
                try:
                    doc = getattr(self, 'do_' + arg).__doc__
                    if doc:
                        self.stdout.write("%s\n" % str(doc))
                        return
                except AttributeError:
                    pass
                self.stdout.write("%s\n" % str(self.nohelp % (arg,)))
                return
            func()
        else:
            names = self.get_names()
            cmds_doc = []
            cmds_undoc = []
            help = {}
            for name in names:
                if name[:5] == 'help_':
                    help[name[5:]] = 1
            names.sort()
            # There can be duplicates if routines overridden
            prevname = ''
            for name in names:
                if name[:3] == 'do_':
                    if name == prevname:
                        continue
                    prevname = name
                    cmd = name[3:]
                    if cmd in help:
                        cmds_doc.append(cmd)
                        del help[cmd]
                    elif getattr(self, name).__doc__:
                        cmds_doc.append(cmd)
                    else:
                        cmds_undoc.append(cmd)
            self.stdout.write("%s\n" % str(self.doc_leader))
            self.print_topics(self.doc_header, cmds_doc, 15, 80)
            self.print_topics(self.misc_header, list(help.keys()), 15, 80)
            self.print_topics(self.undoc_header, cmds_undoc, 15, 80)

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

    def do_add_module_command(self, arg: str):
        """
        Add a module command by module path, function name, module config path, arguments format, and results format.
        """
        args = arg.split(' ')
        args += [None for _ in range(5 - len(args))]
        self.im.add_module_command(pathlib.Path(args[0]), args[1], pathlib.Path(args[2]), args[3] or None,
                                   args[4] or None)

    # todo: try to support minimal help maybe for module commands.
    # todo: support overriding arguments format and results format, like in the module itself.
    def run_module_command(self, function_name: str, arg: str):
        result = getattr(self.im, function_name)(
            *convert_to_types_by_format(self.im.module_commands[function_name]['invocation']['arguments_format'],
                                        *arg.split(' ')))
        print('Result:', ', '.join(str(res) for res in result))

    def do_remove_module_command(self, arg: str):
        """
        Remove a module command by function name.
        """
        self.im.remove_module_command(arg)

    def do_disconnect(self, _=None):
        """
        Disconnect from an Iron Man server.
        """
        del self.im
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
                          'do_add_module_command',
                          'do_remove_module_command',
                          'do_disconnect']
    GENERAL_COMMANDS = ['do_help',
                        'do_exit']


if __name__ == '__main__':
    IronManShell().cmdloop()
