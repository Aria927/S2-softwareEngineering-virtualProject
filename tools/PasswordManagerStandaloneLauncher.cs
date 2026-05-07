using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Reflection;

internal static class PasswordManagerStandaloneLauncher
{
    private const string PayloadResourceName = "payload.zip";
    private const string AppExecutableName = "Password Manager.exe";

    private static int Main(string[] args)
    {
        string extractRoot = Path.Combine(
            Path.GetTempPath(),
            "PasswordManagerStandalone",
            Process.GetCurrentProcess().Id + "-" + DateTime.UtcNow.Ticks);

        try
        {
            Directory.CreateDirectory(extractRoot);
            ExtractPayload(extractRoot);

            string appPath = Path.Combine(extractRoot, AppExecutableName);
            if (!File.Exists(appPath))
            {
                throw new FileNotFoundException("The bundled password manager executable could not be found.", appPath);
            }

            if (args.Length > 0 && string.Equals(args[0], "--self-test", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("Standalone package extracted successfully.");
                return 0;
            }

            ProcessStartInfo startInfo = new ProcessStartInfo
            {
                FileName = appPath,
                WorkingDirectory = extractRoot,
                UseShellExecute = false,
                CreateNoWindow = true
            };

            // This distributable is intentionally local-only, so it runs without needing MySQL.
            startInfo.EnvironmentVariables["PM_STORAGE"] = string.Empty;
            startInfo.EnvironmentVariables["PM_DB_HOST"] = string.Empty;
            startInfo.EnvironmentVariables["PM_DB_PORT"] = string.Empty;
            startInfo.EnvironmentVariables["PM_DB_NAME"] = string.Empty;
            startInfo.EnvironmentVariables["PM_DB_USER"] = string.Empty;
            startInfo.EnvironmentVariables["PM_DB_PASSWORD"] = string.Empty;

            using (Process appProcess = Process.Start(startInfo))
            {
                if (appProcess != null)
                {
                    appProcess.WaitForExit();
                }
            }

            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("Could not start Password Manager: " + ex.Message);
            return 1;
        }
        finally
        {
            TryDeleteDirectory(extractRoot);
        }
    }

    private static void ExtractPayload(string extractRoot)
    {
        Assembly assembly = Assembly.GetExecutingAssembly();
        using (Stream payloadStream = assembly.GetManifestResourceStream(PayloadResourceName))
        {
            if (payloadStream == null)
            {
                throw new InvalidOperationException("The bundled application payload is missing.");
            }

            string zipPath = Path.Combine(extractRoot, "payload.zip");
            using (FileStream zipFile = File.Create(zipPath))
            {
                payloadStream.CopyTo(zipFile);
            }

            ZipFile.ExtractToDirectory(zipPath, extractRoot);
            File.Delete(zipPath);
        }
    }

    private static void TryDeleteDirectory(string path)
    {
        try
        {
            if (Directory.Exists(path))
            {
                Directory.Delete(path, true);
            }
        }
        catch
        {
            // Temporary files are harmless if Windows is still releasing DLL handles.
        }
    }
}
