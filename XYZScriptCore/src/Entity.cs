using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Entity
    {
        public uint ID { get; private set; }

        ~Entity()
        {
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(ID, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(ID, typeof(T));
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            return null;
        }

        public Matrix4 GetTransform()
        {
            Matrix4 mat4Instance;
            GetTransform_Native(ID, out mat4Instance);
            return mat4Instance;
        }

        public void SetTransform(Matrix4 transform)
        {
            SetTransform_Native(ID, ref transform);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(uint entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetTransform_Native(uint entityID, out Matrix4 matrix);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTransform_Native(uint entityID, ref Matrix4 matrix);
    }
}
