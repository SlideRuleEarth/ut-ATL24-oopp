import argparse


def get_make_commands(args):
    cmds = []
    for i in [23, 24, 25, 26, 27]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-x-resolution={i}" classify')
    for i in [1.5, 2.0, 2.5, 3.0]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-vertical-smoothing-sigma={i}" classify')
    for i in [0.01, 0.025, 0.05]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-min-peak-prominence={i}" classify')
    for i in [1, 2, 3, 4, 5]:
        cmds.append(f'make BUILD={args.build} OO_PARAMS="--verbose --oo-min-peak-distance={i}" classify')
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
