# It's a model to test the tonemapping's histogram trim. For more details see the C++/hlsl files

bins = [3, 4, 5, 23, 9, 9, 23, 6]
bin_count = len(bins)

pixel_count = 0
for i in bins:
	pixel_count += i

print(f"Total pixels {pixel_count}")

print(f"Bins {bins}")

# Init prefix_sum array
prefix_sum = [0] * bin_count
for tid in range(0, bin_count):
	prefix_sum[tid] = bins[tid]

# Barrier

# Prefix sum
for i in range(1, bin_count):
	prefix_sum[i] += prefix_sum[i - 1]

print(f"Prefix sum {prefix_sum}")

# Fill the new_bins array
new_bins = [0] * bin_count
for tid in range(0, bin_count):
	new_bins[tid] = bins[tid]

# Trim bins
dark_percent = 0.25 # How many dark pixels to trim
bright_percent = 0.0 # How many bright pixels to trim

dark_pixels_to_trim = int(pixel_count * dark_percent)
bright_pixels_to_trim = int(pixel_count * bright_percent)

print(f"Will trim {dark_pixels_to_trim} dark pixels and {bright_pixels_to_trim} bright pixels (keep {pixel_count - bright_pixels_to_trim})")

for tid in range(0, bin_count):
	pixels_remaining_this_bin = max(0, prefix_sum[tid] - dark_pixels_to_trim)
	pixels_remaining_this_bin = min(pixels_remaining_this_bin, bins[tid])
	new_bins[tid] = pixels_remaining_this_bin

	pixels_to_keep = pixel_count - bright_pixels_to_trim
	pixels_to_remove_this_bin = max(0, prefix_sum[tid] - pixels_to_keep)
	pixels_remaining_this_bin = max(0, new_bins[tid] - pixels_to_remove_this_bin)
	new_bins[tid] = pixels_remaining_this_bin

print(f"New bins {new_bins}")

# Barrier

