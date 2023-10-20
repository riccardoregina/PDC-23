# Importing libraries 
import matplotlib.pyplot as plt 
import numpy as np
  
# Using Numpy to create an array X 
X = np.arange(0, 10, 1) 

# Plotting both the curves simultaneously 
plt.plot([1,2,3,4], color='b', label='SpeedUp') 
plt.plot([1,3,1,3], color='r', label='Efficienza')
plt.plot([0,4,5,2], color='g', label='Overhead')

# Naming the x-axis, y-axis and the whole graph 
plt.xlabel("Numero di processori") 
plt.ylabel("Dimensione del problema") 
plt.title("SpeedUp, Efficienza e Overhead") 
  
# Adding legend, which helps us recognize the curve according to it's color 
plt.legend() 
  
# To load the display window 
plt.show()
