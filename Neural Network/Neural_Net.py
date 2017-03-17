import math

# Connection Class
class Connection:
	def __init__(self,connectedNeuron):
		self.connectedNeuron = connectedNeuron
		self.weight = 0.1
		self.dWeight = 0


# Neuron Class
class Neuron:

	eta = 0.9 #learning rate
	alpha = 0.15 #momentum rate

	def __init__(self,layer):
		
		# Neuron Initialization
		self.dendrons = []
		self.error = 0.0
		self.output = 0.0
		self.gradient = 0.0

		# Input layer has no previous layers
		if(layer is None):
			pass
		# Connect all other layers
		else:
			for neuron in layer:
				con = Connection(neuron)
				self.dendrons.append(con)

	def addError(self,er):
		self.error = self.error + er

	# Sigmoid Activation Function
	def sigmoid(self,x):
		return 1/(1+math.exp(-x))

	def dSigmoid(self,x):
		return x*(1-x)

	# Feed Forward
	def FeedForward(self):
		if(len(self.dendrons)==0):
			return

		sumOfOutputs = 0.0
		for dendron in self.dendrons:
			sumOfOutputs = sumOfOutputs + dendron.connectedNeuron.output*dendron.weight
		self.output = self.sigmoid(sumOfOutputs)

	# Back Propogation
	def BackPropogate(self):
		self.gradient = self.error*self.dSigmoid(self.output)
		for dendron in self.dendrons:
			dendron.dWeight = Neuron.eta*(dendron.connectedNeuron.output*self.gradient) + (Neuron.alpha*dendron.dWeight)
			dendron.weight = dendron.weight + dendron.dWeight
			dendron.connectedNeuron.addError(self.gradient*dendron.weight)
		self.error = 0.0


# Neural Network
class Net:
	def __init__(self,topology):
		self.layers = []

		# Number of neurons in each layer
		for numNeuron in topology:
			layer = []
			for i in range(numNeuron):
				if(len(self.layers)==0):
					neuron = Neuron(None)
				else:
					neuron = Neuron(self.layers[-1])
				layer.append(neuron)

			# Bias Neuron
			layer.append(Neuron(None))
			layer[-1].output = 1
			self.layers.append(layer)

	# Setting inputs of first layer
	def setInputs(self,inputs):
		for i in range(len(inputs)):
			self.layers[0][i].output = inputs[i]

	# Feed Forward
	def FeedForward(self):
		for layer in self.layers:
			for neuron in layer:
				neuron.FeedForward()

	# Back Proprogation
	def BackPropogate(self,targets):
		for i in range(len(targets)):
			self.layers[-1][i].error = targets[i]-self.layers[-1][i].output

		for layer in self.layers[::-1]:
			for neuron in layer:
				neuron.BackPropogate()

	# Calculating errors
	def getError(self,targets):
		avgErr = 0
		for i in range(len(targets)):
			err = targets[i]-self.layers[-1][i].output
			avgErr = avgErr + (err*err)
		avgErr = avgErr/len(targets)
		return avgErr

	def getOutputs(self):
		outputs = []
		for neuron in self.layers[-1]:
			outputs.append(neuron.output)
		return outputs[0:len(outputs)-1];


# Main loop
def main():

	# Initialize Network
	net = Net([2,50,1])
	inputs = [[0,0],[0,1],[1,0],[1,1]]
	outputs = [[0],[1],[1],[1]]
	
	# Training
	while True:
		err = 0
		for i in range(len(inputs)):
			
			print "Input="+str(inputs[i])
			
			net.setInputs(inputs[i])
			net.FeedForward()
			net.BackPropogate(outputs[i])
			err = err + net.getError(outputs[i])
			
			print "Output="+str(net.getOutputs())
			print "Error="+str(net.getError(outputs[i]))+"\n"

		err = err/len(inputs)

		if(err<0.001):
			break

	# Testing
	while True:
		inp1 = input("Enter first input:")
		inp2 = input("Enter second input:")
		inp = [inp1,inp2]
		net.setInputs(inp)
		net.FeedForward()
		print net.getOutputs()

if __name__=="__main__":
	main()



