void twoloops(int a[], int b[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        a[i] = i + 5;
    for (i = 0; i < n; i++)
        b[i] = i + i;
    return;
}