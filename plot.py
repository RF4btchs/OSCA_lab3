"""Графики ускорения S = T1/Tp и эффективности E = S/p по results.csv"""
import csv
import collections
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

data = collections.defaultdict(dict)
with open("results.csv", encoding="utf-8-sig") as f:
    for r in csv.DictReader(f):
        key = (r["case"], int(r["n"]))
        data[key][int(r["threads"])] = float(r["ms"].replace(",", "."))

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.2))
for (case, n), row in data.items():
    ts = sorted(row)
    s = [row[1] / row[t] for t in ts]
    e = [row[1] / row[t] / t for t in ts]
    name = "случайный текст" if case == "random" else "худший случай"
    label = f"{name}, n = " + f"{n:,}".replace(",", " ")
    style = "-" if case == "random" else "--"
    ax1.plot(ts, s, style, marker="o", label=label)
    ax2.plot(ts, e, style, marker="o", label=label)
ts = sorted(next(iter(data.values())))
ax1.plot(ts, ts, "k:", lw=1, label="идеал S = p")
ax1.set_ylim(0, 17)
for ax, title in ((ax1, "Ускорение S = T1 / Tp"), (ax2, "Эффективность E = S / p")):
    ax.set_xscale("log", base=2)
    ax.set_xticks(ts)
    ax.set_xticklabels(ts)
    ax.set_xlabel("максимум потоков p (ключ -t)")
    ax.set_title(title)
    ax.grid(alpha=0.3)
ax1.legend(fontsize=8)
fig.tight_layout()
fig.savefig("img/speedup.png", dpi=150)
print("img/speedup.png")
