import argparse
import numpy as np
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import os.path


def main(args):

    if len(args.input_filenames) == 0:
        raise Exception('No input filenames were specified')

    df = None

    for fn in args.input_filenames:

        if args.verbose:
            print(f'Reading {fn}...')

        tmp = pd.read_csv(fn, sep='\t', engine='pyarrow')

        fns = tmp['filename']
        scores = tmp['Avg']
        model = tmp['model']

        total_nans = scores.isna().sum()

        if total_nans > 0:
            print(f'Warning: found {total_nans} NANs, replacing with 0s')
            scores = scores.fillna(0)

        # Save filenames
        if df is None:
            df = fns.apply(os.path.basename)

        # Save scores
        df = pd.concat([df, scores], axis=1)
        name = model[0]
        df = df.rename(columns={'Avg': name})

    df2 = df.drop(['filename'], axis=1)
    c = df2.corr()

    fig = go.Figure()
    fig.add_trace(go.Heatmap(
        x=c.columns,
        y=c.index,
        z=np.array(c),
        text=c.values,
        texttemplate='%{text:.2f}',
        colorscale='Bluered_r'))
    fig.update_layout(title='Model prediction correlation')
    fig.show()

    df = df.head(50)
    df['average'] = df2.mean(axis=1)
    df = df.sort_values(by=['average'], ascending=False)
    fig2 = px.scatter(df, x='filename', y=df.columns)
    fig2.update_layout(title='Prediction comparison')
    fig2.update_layout(yaxis_title='Score')
    fig2.update_layout(legend_title_text='Model')
    fig2.show()


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument(
        'input_filenames',
        type=str,
        nargs='+')
    parser.add_argument(
        '-v', '--verbose', action='store_true',
        help='Show verbose output')
    args = parser.parse_args()

    main(args)
