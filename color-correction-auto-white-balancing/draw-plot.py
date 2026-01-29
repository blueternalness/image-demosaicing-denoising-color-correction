import matplotlib.pyplot as plt
import numpy as np

means_before = [126.9695, 116.7010, 70.9956]
means_after = [104.4001, 104.4065, 104.3949]

def create_plots(means, title, filename):
    plt.figure(figsize=(8, 5))
    colors = ['red', 'green', 'blue']
    plt.bar(['Red', 'Green', 'Blue'], means, color=colors, alpha=0.7, edgecolor='black')
    plt.axhline(y=np.mean(means), color='black', linestyle='--', label=f'Global Mean (μ)')
    plt.title(title)
    plt.ylabel('Average Intensity (0-255)')
    plt.ylim(0, 255)
    plt.legend()
    plt.savefig(filename)
    plt.show()

create_plots(means_before, "Channel Averages Before AWB (Yellow Tint)", "sea_hist.png")

create_plots(means_after, "Channel Averages After AWB (Balanced)", "sea_awb_hist.png")
