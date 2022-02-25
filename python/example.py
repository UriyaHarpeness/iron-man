import pathlib

from iron_man import IronMan


# Example usage of Iron Man's Python client and their shell equivalents.
def main():
    # >>> connect python/iron_man_config.json
    iron_man = IronMan(pathlib.Path('python/iron_man_config.json'))

    # >>> add_module_command /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so sum
    # python/module_config.json arguments_format=II results_format=I
    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'sum',
        pathlib.Path('python/module_config.json'), 'II', 'I')

    # >>> add_module_command /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so difference
    # python/module_config.json arguments_format=II results_format=I
    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'difference',
        pathlib.Path('python/module_config.json'), 'II', 'I')

    # >>> sum 4 4
    print(iron_man.sum(4, 4)[0])
    # >>> sum 4 400
    print(iron_man.sum(4, 400)[0])
    # >>> sum 1000 24
    print(iron_man.sum(1000, 24)[0])

    # >>> remove_module_command sum
    iron_man.remove_module_command('sum')

    # >>> difference 4 4
    print(iron_man.difference(4, 4)[0])
    # >>> difference 4 400
    print(iron_man.difference(4, 400)[0])
    # >>> difference 1000 24
    print(iron_man.difference(1000, 24)[0])

    # >>> get_file ../main.c
    data = iron_man.get_file('../main.c')

    # >>> run_shell nonexistant
    print('Child exited with', iron_man.run_shell('nonexistant'))
    # >>> run_shell sleep 1
    print('Child exited with', iron_man.run_shell('sleep', ['1']))
    # >>> run_shell ls
    print('Child exited with', iron_man.run_shell('ls'))

    # >>> put_file ../main.c.copy "some text here"
    iron_man.put_file('../main.c.copy', data)

    # >>> run_shell rm ../main.c.copy
    print('Child exited with', iron_man.run_shell('rm', ['../main.c.copy']))
    print(data)

    try:
        # >>> get_file /root
        iron_man.get_file('/root')
    except Exception as e:
        print(e)

    try:
        # >>> get_file ../main.pyyy
        iron_man.get_file('../main.pyyy')
    except Exception as e:
        print(e)

    # >>> suicide
    # iron_man.suicide()

    # >>> stop
    iron_man.stop()

    # >>> disconnect
    # >>> exit


if __name__ == '__main__':
    main()
