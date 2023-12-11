# Importing libraries 
import matplotlib.pyplot as plt 

# Creating x-axis values
x_values3 = [2, 4, 8]
x_values4 = [1, 2, 4, 8]
# Plotting both the curves simultaneously

# tempi
plt.title("Tempo di esecuzione")
plt.plot(x_values4, [7, 6, 5, 3], color='b')
plt.xlabel("Numero di processori") 

#speedup
#plt.title("SpeedUp")
#plt.plot(x_values3, [1.5, 3.5, 6.5], color='r')
#plt.plot(x_values3, [2, 4, 8], color='gray', label='ideale')
#plt.xlabel("Numero di processori")

#overhead
#plt.title("Overhead")
#plt.plot(x_values3, [0.6, 2, 3], color='g')
#plt.plot(x_values3, [0, 0, 0], color='gray', label='ideale')
#plt.xlabel("Numero di processori")

#efficienza
#plt.title("Efficienza")
#plt.plot(x_values3, [0.7, 0.75, 0.8], color='orange')
#plt.plot(x_values3, [1, 1, 1], color='gray', label='ideale')
#plt.xlabel("Numero di processori")

# Adding legend, which helps us recognize the curve according to it's color 
plt.legend()

# To load the display window 
plt.show()
