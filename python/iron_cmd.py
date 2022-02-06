import itertools
import pathlib
import shlex
import traceback
import re
from cmd import Cmd
from typing import Callable, Optional, Tuple

from iron_man import IronMan
from struct_conversion import convert_to_types_by_format


class CustomWrapper:
    def __init__(self, called: Callable):
        self._called = called

    class Wrapped:
        KEYWORD_ARGUMENT_REGEX = re.compile(r'^(\w+)=(.*)$')

        def __init__(self, called: Callable, wrap: Callable[['IronManShell', str], Optional[bool]]):
            self._called = called
            self._wrap = wrap

            doc = self._called.__doc__
            if self._called.__annotations__:
                typing_mapping = {}
                for arg, type_ in self._called.__annotations__.items():
                    typing_mapping[arg] = getattr(type_, '__name__', None) or str(type_)
                typing_doc = 'Typing:\n' + '\n'.join([f'    {arg}: {type_}' for arg, type_ in typing_mapping.items()])
                padded_typing_doc = '\n'.join(
                    [self._called.__doc__.split('\n')[-1] + x for x in typing_doc.split('\n')])
                doc = f'{doc}\n{padded_typing_doc}\n'

            self.__doc__ = doc
            self.__annotations__ = self._called.__annotations__

        def __call__(self, shell: 'IronManShell', *args: str) -> Optional[bool]:
            pass_args = list(itertools.chain(*[shlex.split(arg) for arg in args]))
            pass_kwargs = {}

            for arg_index in range(len(pass_args) - 1, -1, -1):
                match = self.KEYWORD_ARGUMENT_REGEX.match(pass_args[arg_index])
                if match:
                    name, value = match.groups()
                    pass_kwargs[name] = value
                    pass_args.pop(arg_index)

            return self._wrap(shell, *pass_args, **pass_kwargs)

    def __call__(self, wrap: Callable) -> Callable:
        return self.Wrapped(self._called, wrap)


class IronManShell(Cmd):
    intro = 'Welcome to Iron Man\'s shell.   Type help or ? to list commands.\n'
    prompt = '(iron man) '

    def __init__(self):
        super().__init__()
        self.im: Optional[IronMan] = None

    def onecmd(self, line: str) -> Optional[bool]:
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
                if self.im and cmd in self.im._module_commands:
                    return self.run_module_command(self, cmd, arg)
                return self.default(line)

            if f'do_{cmd}' not in self.get_names():
                print(f'Command "{cmd}" is not supported at this state.')
                return

            try:
                if isinstance(func, CustomWrapper.Wrapped):
                    return func(self, arg)
                return func(arg)
            except Exception as e:
                traceback.print_tb(e.__traceback__)
                print(e)

    def do_help(self, arg: str):
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
            # There can be duplicates if routines overridden
            prevname = ''
            for name in sorted(names):
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

    def get_names(self) -> Tuple[str]:
        return self.GENERAL_COMMANDS + (self.CONNECTED_COMMANDS +
                                        tuple(['help_' + command for command in
                                               self.im._module_commands.keys()]) if self.im else
                                        self.DISCONNECTED_COMMANDS)

    def do_connect(self, arg: str):
        """
        Connect to an Iron Man server with given config path.
        """
        self.im = IronMan(pathlib.Path(arg))

    @CustomWrapper(IronMan.get_file)
    def do_get_file(self, path: str):
        print(self.im.get_file(path))

    @CustomWrapper(IronMan.put_file)
    def do_put_file(self, path: str, content: str):
        self.im.put_file(path, content)

    @CustomWrapper(IronMan.run_shell)
    def do_run_shell(self, command: str, *args: str):
        print('Exit code:', self.im.run_shell(command, args))

    @CustomWrapper(IronMan.add_module_command)
    def do_add_module_command(self, module_path: str, function_name: str, module_config_path: str,
                              arguments_format: str = None, results_format: str = None):
        self.im.add_module_command(pathlib.Path(module_path), function_name, pathlib.Path(module_config_path),
                                   arguments_format, results_format)

        def module_command_doc():
            print(f"""
        Run the module command {function_name}.
        Module path: {module_path}, arguments format: {arguments_format}, results format: {results_format}.
            
        Args
            *args: The arguments for the function.
            arguments_format: The struct format of the arguments.
            results_format: The struct format of the results.

        Returns:
            The results of the function.\n""")

        setattr(self, 'help_' + function_name, module_command_doc)

    @CustomWrapper(IronMan.run_module_command)
    def run_module_command(self, function_name: str, *args: str, arguments_format: str = None,
                           results_format: str = None):
        result = getattr(self.im, function_name)(
            *convert_to_types_by_format(
                arguments_format or self.im._module_commands[function_name]['invocation']['arguments_format'], *args),
            arguments_format=arguments_format, results_format=results_format)
        print('Result:', ', '.join(str(res) for res in result))

    @CustomWrapper(IronMan.remove_module_command)
    def do_remove_module_command(self, function_name: str):
        if function_name not in self.im._module_commands:
            print('No such module command:', function_name)
            return

        self.im.remove_module_command(function_name)
        delattr(self, 'help_' + function_name)

    def do_disconnect(self, _=None):
        """
        Disconnect from an Iron Man server.
        """
        del self.im
        self.im = None

    @CustomWrapper(IronMan.stop)
    def do_stop(self):
        self.im.stop()
        self.do_disconnect()

    @CustomWrapper(IronMan.suicide)
    def do_suicide(self):
        self.im.suicide()
        self.do_disconnect()

    def do_exit(self, _=None):
        """
        Exit the Iron Man shell.
        """
        self.do_disconnect()
        return True

    def __del__(self):
        self.do_exit()

    DISCONNECTED_COMMANDS = ('do_connect',)
    CONNECTED_COMMANDS = ('do_get_file',
                          'do_put_file',
                          'do_run_shell',
                          'do_add_module_command',
                          'do_remove_module_command',
                          'do_disconnect',
                          'do_stop',
                          'do_suicide')
    GENERAL_COMMANDS = ('do_help',
                        'do_exit')


if __name__ == '__main__':
    IronManShell().cmdloop()
