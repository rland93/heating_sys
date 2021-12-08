import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import itertools

# read TCS data from file and return the 3 arrays with it.
def read_tcsdata_from_file(fname:str):
    df = pd.read_csv(fname, sep=",")
    df = df.rename(columns=lambda x: x.strip())
    return df

if __name__ == "__main__":
    df = read_tcsdata_from_file("tcsdata4")
    print(df.columns)
    fig, ax = plt.subplots(nrows=2, sharex=True)
    ax[0].plot(df["time"], df["T"], label="T")
    ax[0].axhline(y=df["Set Point"][0], color="red", label="Setpoint", linestyle="--")
    ax[0].legend()
    ax[0].set_ylabel("Temperature")
    ax[1].plot(df["time"], df["Duty Factor"], label="Duty Factor")
    ax[1].legend()
    ax[1].set_ylabel("Duty Factor")
    ax[1].set_ylim(0, 1.1)
    ax[1].set_xlabel("Time [s]")
    plt.show()