import argparse
import numpy as np
import pickle
import os
import warnings

warnings.filterwarnings("ignore")

from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import top_k_accuracy_score
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix
from tqdm import trange

parser = argparse.ArgumentParser()
parser.add_argument("--data_file", default="data", type=str)
parser.add_argument("--n", default=10, type=int)
parser.add_argument("--test_size", default=0.25, type=float)
opts = parser.parse_args()

def get_data(path):
    # Data preparation
    traces = []
    labels = []

    if os.path.isdir(path):
        # If directory, find all .pkl files
        filepaths = [os.path.join(path, x) for x in os.listdir(path) if x.endswith(".pkl")]
    elif os.path.isfile(path):
        # If single file, just use this one
        filepaths = [path]
    else:
        raise RuntimeError

    for file in filepaths:
        f = open(file, "rb")

        while True:
            try:
                data = pickle.load(f)
                traces_i, labels_i = data[0], data[1]

                if isinstance(traces_i[0], list):
                    traces.extend(traces_i)
                else:
                    traces.append(traces_i)

                labels.append(labels_i)
            except EOFError:
                break

    traces = np.array(traces)

    # Convert labels from domain names to ints
    domains = list(set(labels))
    int_mapping = {x: i for i, x in enumerate(domains)}
    labels = [int_mapping[x] for x in labels]
    labels = np.array(labels)

    return traces, labels, domains


def get_accs(X, y, domains):
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=opts.test_size, stratify=y)

    clf = RandomForestClassifier()
    clf = clf.fit(X_train, y_train)

    y_probs = clf.predict_proba(X_test)
    top1 = top_k_accuracy_score(y_test, y_probs, k=1)
    top5 = top_k_accuracy_score(y_test, y_probs, k=5)

    return [top1, top5]

def plot_confusion_matrix(X, y, domains, out_file="confusion_matrix.png"):
    X_train, X_test, y_train, y_test = train_test_split(
        X,
        y,
        test_size=opts.test_size,
        stratify=y,
        random_state=0,
    )

    clf = RandomForestClassifier(random_state=0)
    clf.fit(X_train, y_train)

    y_pred = clf.predict(X_test)

    n_classes = len(domains)
    labels = np.arange(n_classes)

    cm = confusion_matrix(y_test, y_pred, labels=labels)

    # Normalize by row: each website sums to 1
    row_sums = cm.sum(axis=1, keepdims=True)
    cm_norm = np.divide(
        cm,
        row_sums,
        out=np.zeros_like(cm, dtype=float),
        where=row_sums != 0,
    )

    # Automatically adjust figure size
    fig_size = max(6, min(18, n_classes * 0.18))
    fig, ax = plt.subplots(figsize=(fig_size, fig_size))

    im = ax.imshow(cm_norm, interpolation="nearest", cmap="Blues", vmin=0, vmax=1)

    ax.set_xlabel("Prediction")
    ax.set_ylabel("Website")

    # Automatically adjust tick density
    if n_classes <= 30:
        tick_step = 1
    elif n_classes <= 60:
        tick_step = 5
    elif n_classes <= 120:
        tick_step = 10
    elif n_classes <= 250:
        tick_step = 25
    else:
        tick_step = 50

    ticks = np.arange(0, n_classes, tick_step)
    ax.set_xticks(ticks)
    ax.set_yticks(ticks)

    # Show labels as 1..N, like your example
    ax.set_xticklabels(ticks + 1)
    ax.set_yticklabels(ticks + 1)

    ax.set_xlim(-0.5, n_classes - 0.5)
    ax.set_ylim(n_classes - 0.5, -0.5)

    fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)

    plt.tight_layout()
    plt.savefig(out_file, dpi=300)
    plt.close()

    print(f"Saved confusion matrix to {out_file}")
    
print(f"Analyzing results from {opts.data_file}")

X, y, domains = get_data(opts.data_file)
accs = np.array([get_accs(X, y, domains) for _ in trange(opts.n)])
print()

top1 = accs[:, 0].mean()
top1_std = accs[:, 0].std()

top5 = accs[:, 1].mean()
top5_std = accs[:, 1].std()

print(f"Number of traces: {len(X)}")
print()
print("top1 accuracy: {:.1f}% (+/- {:.1f}%)".format(top1 * 100, top1_std * 100))
print("top5 accuracy: {:.1f}% (+/- {:.1f}%)".format(top5 * 100, top5_std * 100))

plot_confusion_matrix(
    X,
    y,
    domains,
    out_file=os.path.join(opts.data_file, "fig8.png")
    if os.path.isdir(opts.data_file)
    else "fig8.png",
)
