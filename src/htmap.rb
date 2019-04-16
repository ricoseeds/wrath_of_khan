
def p_arr(arr)
    width = arr.flatten.max.to_s.size+2
    for i in (0..arr.length - 1)
        for j in (0..arr.length - 1)
            print arr[i][j].to_s.rjust(width + 2) + " "
        end
        puts ""
    end
end
def getrnd(range)
    rand -range.to_f..range.to_f
end

n = 8
arr = Array.new(2**n + 1) { Array.new(2**n +1) { 0 } }
n_augmented = 2**n 
arr[0][0] = rand(10 + 1)
arr[0][n_augmented] = rand(10 + 1)
arr[n_augmented][0] = rand(10 + 1)
arr[n_augmented][n_augmented] = rand(10 + 1)


a = arr
n_augmented_copy = n_augmented
range = 50.0
while n_augmented > 1
    i = 0
    while(i < n_augmented_copy)
        j = 0
        while(j < n_augmented_copy)
            a[i][j+(n_augmented)/2] = (a[i][j] + a[i][j+n_augmented]) / 2.0  + getrnd(range)
            a[i + (n_augmented) / 2][j + n_augmented] = (a[i][j + n_augmented] + a[i + n_augmented][j + n_augmented]) / 2.0 + getrnd(range)
            a[i + n_augmented][j +( n_augmented) / 2] = (a[i + n_augmented][j] + a[i + n_augmented][j + n_augmented] )/ 2.0 + getrnd(range)
            a[i + (n_augmented) / 2][j]  = (a[i + n_augmented][j] + a[i][j] )/ 2.0 + getrnd(range)
            a[i + (n_augmented) / 2][j +( n_augmented) / 2] = ((a[i][j+(n_augmented)/2] + a[i + (n_augmented) / 2][j + n_augmented] + a[i + n_augmented][j +( n_augmented) / 2]  + a[i + (n_augmented) / 2][j]))/ 4.0  + getrnd(range)
            j = j + n_augmented    
        end
        i = i + n_augmented
    end
    range = range / 2
    n_augmented = n_augmented / 2
end

board = n_augmented_copy + 1
c = 0 
k = board + 1
#Print vertices 
File.open("mountain.obj", 'w') { |file| 
    for i in 0..a.length - 1
        for j in 0..a.length - 1
            file.write("v " + j.to_f.to_s + " " + a[i][j].to_f.to_s + " " + i.to_f.to_s+ "\n")
        end
    end

    for i in 0..a.length - 2
        for i in 0..a.length - 2
            file.write("f " + (c + i + 1).to_s  + " " + k.to_s + " " + (c + i + 2).to_s + "\n")
            file.write("f " + (c + i + 2).to_s + " "  + (k).to_s + " " + (k+1).to_s  + "\n")
            k=k+1
        end
        k = k +1
        c += board 
    end
}
