def sumlist(x):
    if x == []:
        return 0
    else:
        return x[0] + sumlist(x[1:])
print(sumlist([1, 2, 7, 4]))
