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
    print('Summing 4 and 4 gives:', iron_man.sum(4, 4)[0])
    # >>> sum 4 400
    print('Summing 4 and 400 gives:', iron_man.sum(4, 400)[0])
    # >>> sum 1000 24
    print('Summing 1000 and 24 gives:', iron_man.sum(1000, 24)[0])

    # >>> remove_module_command sum
    iron_man.remove_module_command('sum')

    # >>> difference 4 4
    print('The difference between 4 and 4 is:', iron_man.difference(4, 4)[0])
    # >>> difference 4 400
    print('The difference between 4 and 400 is:', iron_man.difference(4, 400)[0])
    # >>> difference 1000 24
    print('The difference between 1000 and 24 is:', iron_man.difference(1000, 24)[0])

    # >>> get_file /etc/hosts
    data = iron_man.get_file('/etc/hosts')

    # >>> run_shell nonexistant
    print('Running "nonexistant", exited with', iron_man.run_shell('nonexistant'))
    # >>> run_shell sleep 1
    print('Running "sleep 1", exited with', iron_man.run_shell('sleep', ['1']))
    # >>> run_shell ls -l /proc
    print('Running "ls -l /proc", exited with', iron_man.run_shell('ls', ['-l', '/proc']))

    # >>> put_file /tmp/hosts_copy "some text here"
    iron_man.put_file('/tmp/hosts_copy', data)

    # >>> run_shell rm /tmp/hosts_copy
    print('Child exited with', iron_man.run_shell('rm', ['/tmp/hosts_copy']))
    print(data)

    try:
        # >>> get_file /root
        print('Getting file "/root"')
        iron_man.get_file('/root')
    except Exception as e:
        print(e)

    try:
        # >>> get_file nonexistant
        print('Getting file "nonexistant"')
        iron_man.get_file('nonexistant')
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
