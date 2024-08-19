import argparse


def get_make_commands(args):
    cmds = []
    for i in [2.0, 2.5, 3.0, 3.5, 4.0]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-surface-n-stddev={i}" classify')
    for i in [2.0, 2.5, 3.0, 3.5, 4.0]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-bathy-n-stddev={i}" classify')
    for i in [2, 3, 4, 5, 6, 7, 8]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-min-surface-photons-per-window={i}" classify')
    for i in [2, 3, 4, 5, 6, 7, 8]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-min-bathy-photons-per-window={i}" classify')
    return cmds


def main(args):
    cmds = get_make_commands(args)
    for i in cmds:
        print(f'rm ./predictions/*')
        print(i)
        print(f'make score')
        print(f'echo "command = {i}" >> search_results.txt')
        print(f'cat ./no_surface_micro_oopp.txt >> search_results.txt')
        print(f'cat ./micro_oopp.txt >> search_results.txt')


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-b', '--build',
        type=str,
        default='debug',
        help='Build string [debug|release]')
    parser.add_argument(
        '-v', '--verbose', action='store_true',
        help='Show verbose output')
    args = parser.parse_args()

    main(args)
