import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv('original_histogram.csv')
df.plot(x='Intensity', y='Pixel_Count')
#plt.show()
plt.savefig('original_histogram.png')

df = pd.read_csv('methodA_transfer_function.csv')
df.plot(x='Input_Intensity', y='Output_Intensity')
#plt.show()
plt.savefig('methodA_transfer_function.png')

df = pd.read_csv('methodB_cdf.csv')
df.plot(x='Intensity', y='Cumulative_Count')
#plt.show()
plt.savefig('methodB_cdf.png')
