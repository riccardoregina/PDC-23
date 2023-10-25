# Importing libraries 
import matplotlib.pyplot as plt 

# Creating x-axis values
x_values = [1, 2, 4, 8]  

# Plotting both the curves simultaneously (random values at the moment)
plt.plot(x_values, [1,2.32,3.43,4.67], color='b', label='SpeedUp') 
plt.plot(x_values, [1,3,1,3], color='r', label='Efficienza')
plt.plot(x_values, [20,4,5,2], color='g', label='Overhead')

# Naming the x-axis, y-axis and the whole graph
plt.xlabel("Numero di processori") 
plt.ylabel("Dimensione del problema") 
plt.title("SpeedUp, Efficienza e Overhead") 
  
# Adding legend, which helps us recognize the curve according to it's color 
plt.legend() 

# To load the display window 
plt.show()
