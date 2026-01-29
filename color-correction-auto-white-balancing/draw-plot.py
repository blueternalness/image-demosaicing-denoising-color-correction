import matplotlib.pyplot as plt
import numpy as np

# Data points extracted from your C++ code
means_before = [126.9695, 116.7010, 70.9956] # Example: R, G, B
means_after = [104.4001, 104.4065, 104.3949]  # Example: Target mu

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

# (1) Plot Before AWB
create_plots(means_before, "Channel Averages Before AWB (Yellow Tint)", "sea_hist.png")

# (2) Plot After AWB
create_plots(means_after, "Channel Averages After AWB (Balanced)", "sea_awb_hist.png")
