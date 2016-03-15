
def is_power_of_two(number):
	return ((number & (number - 1)) == 0) and number != 0