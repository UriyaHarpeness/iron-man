import pathlib

from iron_man import IronMan


def main():
    iron_man = IronMan(pathlib.Path('iron_man_config.json'))

    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'sum',
        pathlib.Path('module_config.json'), 'II', 'I')
    iron_man.add_module_command(
        pathlib.Path('/home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so'), 'difference',
        pathlib.Path('module_config.json'), 'II', 'I')
    print(iron_man.sum(4, 4)[0])
    print(iron_man.sum(4, 400)[0])
    print(iron_man.sum(1000, 24)[0])
    iron_man.remove_module_command('sum')
    print(iron_man.difference(4, 4)[0])
    print(iron_man.difference(4, 400)[0])
    print(iron_man.difference(1000, 24)[0])

    data = iron_man.get_file('../main.c')
    print('Child exited with', iron_man.run_shell('wot'))
    print('Child exited with', iron_man.run_shell('sleep', ['1']))
    print('Child exited with', iron_man.run_shell('ls'))
    iron_man.put_file('../main.c.copy', data)
    try:
        iron_man.get_file('/root')
        iron_man.get_file('../main.pyyy')
    except Exception as e:
        print(e)
    print(iron_man.get_file('../main.c'))
    # iron_man.suicide()
    iron_man.stop()


if __name__ == '__main__':
    main()
