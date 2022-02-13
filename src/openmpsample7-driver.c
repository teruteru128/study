
void bin_search(int pos, int n, char *state);

int main(int argc, char const *argv[])
{
    char state[1024] = "";
    bin_search(0, 20, state);
    return 0;
}
