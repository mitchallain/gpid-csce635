#! /usr/bin/env python

##########################################################################################
# annalysis.py
#
# Plotting methods for the PIDNN controller
#
# NOTE:
#
# Created: April 28, 2017
#   - Mitchell Allain
#   - allain.mitch@gmail.com
#
# Modified:
#   *
#
##########################################################################################

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.animation import FuncAnimation
import scipy.optimize


def load_data_from_stamp(stamp):
    trial = pd.read_csv('xuesu0_out_' + stamp + '.csv')
    trial_meta = pd.read_csv('xuesu0_meta_' + stamp + '.csv')
    return trial, trial_meta


def load_data_from_fname(fname):
    ''' Returns trial and trial_meta dataframes from inferred timestamp '''
    trial = pd.read_csv(fname)
    trial_meta = pd.read_csv('meta'.join(fname.split('out')))
    return trial, trial_meta


def P_trial_fix(trial):
    ''' Fixes trials run by sim_custom.m with data under wrong headers'''
    trial['em_y'] = trial['r']
    trial['em_x'] = trial['wt3']
    trial['y'] = trial['wt2']
    trial['r'] = trial['wt1']
    return trial


def trim_time(tr):
    ''' Trims whole dataframe to last non-null value '''
    if tr['t'].isnull().any():
        t_stop = np.where(tr['t'].isnull().values)[0]
        t_stop = t_stop[0]
        tr = tr.drop(range(t_stop, len(tr)))
    return tr


def view_trial(trial, trial_meta, ax, scat=False, label='Path', ls='-', leg=False, path=True):
    start = (trial['em_x'].values[0], trial['em_y'].values[0])
    end = (float(trial_meta['em2vic_x'].values[0]), float(trial_meta['em2vic_y'].values[0]))

    ax.plot(trial['em_x'], trial['em_y'], label=label, linestyle=ls)

    if scat:
        ax.scatter(*start, marker='x', s=80, label='Initial Pos')
        ax.scatter(*end, marker='o', s=80, label='Victim')

    ax.axis('equal')

    plt.xlabel('X (m)')
    plt.ylabel('Y (m)')

    if path:
        start = (trial['em_x'][0], trial['em_y'][0])
        end = (trial_meta['em2vic_x'], trial_meta['em2vic_y'])
        ax.plot(*zip(start, end), linestyle=':', color='gray', label='Path')

    if leg:
        plt.legend(loc='best')






# def animate_many(trials, trials_meta):
#     fig = plt.figure(figsize=(6, 4))

#     plt.xlabel('X (m)')
#     plt.ylabel('Y (m)')

#     start = (trials[0]['em_x'][0], trials[0]['em_y'][0])
#     end = (float(trials_meta[0]['em2vic_x'][0]), float(trials_meta[0]['em2vic_y'][0]))
#     plt.scatter(*start, marker='x', label='Initial Pos')
#     plt.scatter(*end, marker='o', label='Victim')
#     plt.plot(*zip(start, end), linestyle='--', color='b')

#     paths = []
#     for i in range(len(trials)):
#         path, = plt.plot([], [])
#         paths.append(path)

#     plt.legend(loc='lower left')
#     plt.title('EMILY Paths')

#     plt.tight_layout()

#     def update(i):
#         for j, tr in enumerate(trials):
#             i = np.clip(i, 0, len(tr))
#             paths[j].set_data(tr['em_x'][:i].values, tr['em_y'][:i].values)
#         plt.axis('equal')

#     len_anim = max([len(trial) for trial in trials])

#     anim = FuncAnimation(fig, update,
#                          frames=np.arange(0, len_anim/10)*10,
#                          interval=200)

#     anim.save('fig/many_paths.gif', dpi=150, writer='imagemagick')


def plot_weights(trials, trial_metas, window=10, title='Weights'):
    wts = np.zeros((len(trials), 4))
    wts[:, 0] = np.arange(len(trials))

    for i, tr in enumerate(trials):
        tr = tr.set_index('t')
        wts[i, 1:] = tr[['wt1', 'wt2', 'wt3']].values[-1]

    wtsdf = pd.DataFrame(wts)
    wtsdf.columns = ['trial', 'wt1', 'wt2', 'wt3']
    wtsdf = wtsdf.set_index('trial')
    wts_smooth = wtsdf[['wt1', 'wt2', 'wt3']].rolling(min_periods=1, window=window).mean()

    plt.figure(figsize=(6, 6))
    plt.ylabel('Weight')

    ax1 = plt.subplot(3, 1, 1)
    plt.plot(wts_smooth['wt1'])
    plt.title(title)
    plt.ylabel('$k_P$', rotation='horizontal')
    ax1.yaxis.labelpad = 15

    ax2 = plt.subplot(3, 1, 2)
    plt.plot(wts_smooth['wt2'])
    plt.ylabel('$k_I$', rotation='horizontal')
    ax2.yaxis.labelpad = 15

    ax3 = plt.subplot(3, 1, 3)
    plt.plot(wts_smooth['wt3'])
    plt.ylabel('$k_D$', rotation='horizontal')
    ax3.yaxis.labelpad = 15

    plt.xlabel('Trial $n$')

    plt.tight_layout()

    return wtsdf, wts_smooth


def plot_cte(trials, trial_metas, window=25):
    ctes = np.zeros((len(trials), 2))
    ctes[:, 0] = np.arange(len(trials)) + 1

    for i, (tr, trm) in enumerate(zip(trials, trial_metas)):
        ctes[i, 1] = straight_mean_cte(tr, trm)

    ctedf = pd.DataFrame(ctes)
    ctedf.columns = ['Trial No.', 'Mean CTE']
    ctedf = ctedf.set_index('Trial No.')

    ctedf['Smoothed Mean CTE'] = ctedf['Mean CTE'].rolling(min_periods=3, window=window).mean()
    ctedf.plot()
    plt.ylabel('Mean Cross-Track Error (CTE)')

    return ctedf


def straight_path(x, trial, trial_meta):
    start = (trial['em_x'][0], trial['em_y'][0])
    end = (float(trial_meta['em2vic_x'][0]), float(trial_meta['em2vic_y'][0]))

    diff = np.array(start) - np.array(end)
    m = diff[1]/diff[0]

    return np.array([x, x*m])


def norm(s, path, x, y, trial, trial_meta):
    return np.linalg.norm(path(s, trial, trial_meta) - np.array([x, y]))


def cross_track_error(path, objective, trial, trial_meta, x, y):
    res = scipy.optimize.minimize(objective, x0=5, args=(path, x, y, trial, trial_meta))
    return res.fun


def trial_cross_track_error(trial, trial_meta):
    trial['xte'] = np.vectorize(cross_track_error, excluded=(0, 1, 2, 3))(straight_path, norm, trial,
                                                                          trial_meta, trial['em_x'], trial['em_y'])
    return trial['xte'].sum() / trial['t'].values[-1]


def straight_cte(p_ev, p_ev_i):
    return np.abs(np.cross(p_ev, p_ev_i)) / np.linalg.norm(p_ev_i)


def straight_mean_cte(trial, trial_meta):
    ''' Mean cross-track error for straight paths
        NOTE: will not work if emily is not starting at (0, 0)
    '''
    p_v = trial_meta[['em2vic_x', 'em2vic_y']].values[0]
    p_e = trial[['em_x', 'em_y']].values
    p_ev = p_v - p_e
#     cte = np.vectorize(straight_cte, excluded=('p_ev_i'))(p_ev, p_v)
    cte = straight_cte(p_ev, p_v)
    return np.mean(cte)


# def straight_mean_cte(trial, trial_meta):
#     ''' Mean cross-track error for straight paths
#         NOTE: will not work if emily is not starting at (0, 0)
#     '''
#     p_v = trial_meta[['em2vic_x', 'em2vic_y']].values[0].astype('float64')
#     p_e = trial[['em_x', 'em_y']].values
#     p_ev = p_v - p_e
# #     cte = np.vectorize(straight_cte, excluded=('p_ev_i'))(p_ev, p_v)
#     cte = straight_cte(p_ev, p_v)
#     return np.mean(cte)
