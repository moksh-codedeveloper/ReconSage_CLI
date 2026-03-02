namespace ITor
{
    public interface ITorController
    {
        Task RotateAsync(CancellationToken token = default);
    }
}